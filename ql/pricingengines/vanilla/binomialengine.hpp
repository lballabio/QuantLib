/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl
 Copyright (C) 2007 Affine Group Limited

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binomialengine.hpp
    \brief Binomial option engine
*/

#ifndef quantlib_binomial_engine_hpp
#define quantlib_binomial_engine_hpp

#include <ql/methods/lattices/binomialtree.hpp>
#include <ql/methods/lattices/bsmlattice.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/vanilla/discretizedvanillaoption.hpp>
#include <ql/pricingengines/greeks.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>

namespace QuantLib {

    //! Pricing engine for vanilla options using binomial trees
    /*! \ingroup vanillaengines

        \test the correctness of the returned values is tested by
              checking it against analytic results.

        \todo Greeks are not overly accurate. They could be improved
              by building a tree so that it has three points at the
              current time. The value would be fetched from the middle
              one, while the two side points would be used for
              estimating partial derivatives.
    */

    /*
        As of 24 May 2020, I have updated the file to incorporate vega
        computations.
        The details' index are as follow:
        - Create a new handle for the flat volatilty, stressed by a factor of
            0.001.
        - Create a new BS process that with every input the same but the stressed
            volatility.
        - Create a new tree based in the new BS process.
        - Create a new BS lattice based on the new tree.
        - The new option is initialized.
        - Rollback and compute the PV for the new option.
        - Line compute the vega by (Option2.NPV() - Option.NPV())/0.001, being Option 2 the
            one with the stressed volaitility.
        - Add vega to the output.

    */
    template <class T>
    class BinomialVanillaEngine : public VanillaOption::engine {
      public:
        BinomialVanillaEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps)
        : process_(process), timeSteps_(timeSteps) {
            QL_REQUIRE(timeSteps >= 2,
                       "at least 2 time steps required, "
                       << timeSteps << " provided");
            registerWith(process_);
        }
        void calculate() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_;
    };


    // template definitions

    template <class T>
    void BinomialVanillaEngine<T>::calculate() const {

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Calendar volcal = process_->blackVolatility()->calendar();

        Real s0 = process_->stateVariable()->value();
        QL_REQUIRE(s0 > 0.0, "negative or null underlying given");
        Volatility v = process_->blackVolatility()->blackVol(
            arguments_.exercise->lastDate(), s0);
        
        Date maturityDate = arguments_.exercise->lastDate();
        Rate r = process_->riskFreeRate()->zeroRate(maturityDate,
            rfdc, Continuous, NoFrequency);
        Rate q = process_->dividendYield()->zeroRate(maturityDate,
            divdc, Continuous, NoFrequency);
        Date referenceDate = process_->riskFreeRate()->referenceDate();

        // binomial trees with constant coefficient
        Handle<YieldTermStructure> flatRiskFree(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, r, rfdc)));
        Handle<YieldTermStructure> flatDividends(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, q, divdc)));
        Handle<BlackVolTermStructure> flatVol(
            ext::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, volcal, v, voldc)));
        Handle<BlackVolTermStructure> flatVol2(
            ext::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, volcal, v + 0.001, voldc))); //Second handle for stressed volatility 

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Time maturity = rfdc.yearFraction(referenceDate, maturityDate);

        ext::shared_ptr<StochasticProcess1D> bs(
                         new GeneralizedBlackScholesProcess(
                                      process_->stateVariable(),
                                      flatDividends, flatRiskFree, flatVol));

        ext::shared_ptr<StochasticProcess1D> bs2(
            new GeneralizedBlackScholesProcess(
                process_->stateVariable(),
                flatDividends, flatRiskFree, flatVol2));                 // Second BS process with stressed volatility

        TimeGrid grid(maturity, timeSteps_);

        ext::shared_ptr<T> tree(new T(bs, maturity, timeSteps_,
                                        payoff->strike()));

        ext::shared_ptr<T> tree2(new T(bs2, maturity, timeSteps_,
            payoff->strike()));                                          // Second tree with the new BS based on the stressed volatility

        ext::shared_ptr<BlackScholesLattice<T> > lattice(
            new BlackScholesLattice<T>(tree, r, maturity, timeSteps_));

        ext::shared_ptr<BlackScholesLattice<T> > lattice2(
            new BlackScholesLattice<T>(tree2, r, maturity, timeSteps_)); // Second lattice with the volatility stressed tree and BS process

        //Initialize the two option objects 
        DiscretizedVanillaOption option(arguments_, *process_, grid);
        DiscretizedVanillaOption option2(arguments_, *process_, grid); 

        option.initialize(lattice, maturity);
        option2.initialize(lattice2, maturity);

        // Partial derivatives calculated from various points in the
        // binomial tree 
        // (see J.C.Hull, "Options, Futures and other derivatives", 6th edition, pp 397/398)

        // Rollback to third-last step, and get underlying prices (s2) &
        // option values (p2) at this point
        option.rollback(grid[2]);
        Array va2(option.values());
        QL_ENSURE(va2.size() == 3, "Expect 3 nodes in grid at second step");
        Real p2u = va2[2]; // up
        Real p2m = va2[1]; // mid
        Real p2d = va2[0]; // down (low)
        Real s2u = lattice->underlying(2, 2); // up price
        Real s2m = lattice->underlying(2, 1); // middle price
        Real s2d = lattice->underlying(2, 0); // down (low) price

        // calculate gamma by taking the first derivate of the two deltas
        Real delta2u = (p2u - p2m)/(s2u-s2m);
        Real delta2d = (p2m-p2d)/(s2m-s2d);
        Real gamma = (delta2u - delta2d) / ((s2u-s2d)/2);

        // Rollback to second-last step, and get option values (p1) at
        // this point
        option.rollback(grid[1]);
        Array va(option.values());
        QL_ENSURE(va.size() == 2, "Expect 2 nodes in grid at first step");
        Real p1u = va[1];
        Real p1d = va[0];
        Real s1u = lattice->underlying(1, 1); // up (high) price
        Real s1d = lattice->underlying(1, 0); // down (low) price

        Real delta = (p1u - p1d) / (s1u - s1d);

        // Finally, rollback to t=0
        option.rollback(0.0);
        option2.rollback(0.0);
        Real p0 = option.presentValue();
        Real p0_2 = option2.presentValue(); // Compute the PV of the option with the stressed volatility

        Real vega = (p0_2 - p0) / 0.001;

        // Store results
        results_.value = p0;
        results_.delta = delta;
        results_.gamma = gamma;
        results_.vega = vega;
        results_.theta = blackScholesTheta(process_,
                                           results_.value,
                                           results_.delta,
                                           results_.gamma);
    }

}


#endif
