/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Thema Consulting SA

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

/*! \file binomialbarrierengine.hpp
    \brief Binomial Barrier option engine
*/

#ifndef quantlib_binomial_barrier_engine_hpp
#define quantlib_binomial_barrier_engine_hpp

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/lattices/binomialtree.hpp>
#include <ql/methods/lattices/bsmlattice.hpp>
#include <ql/pricingengines/barrier/discretizedbarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <utility>

namespace QuantLib {

    //! Pricing engine for barrier options using binomial trees
    /*! \ingroup barrierengines

        \note Timesteps for Cox-Ross-Rubinstein trees are adjusted using Boyle and Lau algorithm.
              See Journal of Derivatives, 1/1994,
              "Bumping up against the barrier with the binomial method"

        \test the correctness of the returned values is tested by
              checking it against analytic european results.
    */
    template <class T, class D>
    class BinomialBarrierEngine : public BarrierOption::engine {
      public:
        /*! The maxTimeSteps parameter is used to limit timeSteps when
            using Boyle-Lau optimization. If zero (the default) the
            maximum number of steps is calculated by an heuristic:
            anything when < 1000, otherwise no more than 5*timeSteps.
            If maxTimeSteps is equal to timeSteps, Boyle-Lau is
            disabled.  Likewise if the lattice is not
            CoxRossRubinstein Boyle-Lau is disabled and maxTimeSteps
            ignored.
        */
        BinomialBarrierEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                              Size timeSteps,
                              Size maxTimeSteps = 0)
        : process_(std::move(process)), timeSteps_(timeSteps), maxTimeSteps_(maxTimeSteps) {
            QL_REQUIRE(timeSteps>0,
                       "timeSteps must be positive, " << timeSteps <<
                       " not allowed");
            QL_REQUIRE(maxTimeSteps==0 || maxTimeSteps>=timeSteps,
                       "maxTimeSteps must be zero or "
                       "greater than or equal to timeSteps, "
                       << maxTimeSteps << " not allowed");
            if (maxTimeSteps_==0)
               maxTimeSteps_ = std::max( (Size)1000, timeSteps_*5);
            registerWith(process_);
        }
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_;
        Size maxTimeSteps_;
    };


    // template definitions

    template <class T, class D>
    void BinomialBarrierEngine<T,D>::calculate() const {

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

        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Time maturity = rfdc.yearFraction(referenceDate, maturityDate);

        ext::shared_ptr<StochasticProcess1D> bs(
                         new GeneralizedBlackScholesProcess(
                                      process_->stateVariable(),
                                      flatDividends, flatRiskFree, flatVol));

        // correct timesteps to ensure a (local) minimum, using Boyle and Lau
        // approach. See Journal of Derivatives, 1/1994,
        // "Bumping up against the barrier with the binomial method"
        // Note: this approach works only for CoxRossRubinstein lattices, so
        // is disabled if T is not a CoxRossRubinstein or derived from it.
        Size optimum_steps = timeSteps_;
        if (boost::is_base_of<CoxRossRubinstein, T>::value && 
            maxTimeSteps_ > timeSteps_ && s0 > 0 && arguments_.barrier > 0) {
            Real divisor;
            if (s0 > arguments_.barrier)
               divisor = std::pow(std::log(s0 / arguments_.barrier), 2);
            else
               divisor = std::pow(std::log(arguments_.barrier / s0), 2);
            if (!close(divisor,0)) {
                for (Size i=1; i < timeSteps_ ; ++i) {
                    Size optimum = Size(( i*i * v*v * maturity) / divisor);
                    if (timeSteps_ < optimum) {
                        optimum_steps = optimum;
                        break; // found first minimum with iterations>=timesteps
                    }
                }
            }

            if (optimum_steps > maxTimeSteps_) 
               optimum_steps = maxTimeSteps_; // too high, limit
        }

        TimeGrid grid(maturity, optimum_steps);

        ext::shared_ptr<T> tree(new T(bs, maturity, optimum_steps,
                                        payoff->strike()));

        ext::shared_ptr<BlackScholesLattice<T> > lattice(
            new BlackScholesLattice<T>(tree, r, maturity, optimum_steps));

        D option(arguments_, *process_, grid);
        option.initialize(lattice, maturity);

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
        Real p0 = option.presentValue();

        // Store results
        results_.value = p0;
        results_.delta = delta;
        results_.gamma = gamma;
        // theta can be approximated by calculating the numerical derivative
        // between mid value at third-last step and at t0. The underlying price
        // is the same, only time varies.
        results_.theta = (p2m - p0) / grid[2];
    }

}


#endif
