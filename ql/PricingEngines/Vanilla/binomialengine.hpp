
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003, 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binomialengine.hpp
    \brief Binomial option engine
*/

#ifndef quantlib_binomial_engine_hpp
#define quantlib_binomial_engine_hpp

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/PricingEngines/Vanilla/discretizedvanillaoption.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    //! Pricing engine for vanilla options using binomial trees
    template <class TreeType>
    class BinomialVanillaEngine : public VanillaOption::engine {
      public:
        BinomialVanillaEngine(Size timeSteps)
        : timeSteps_(timeSteps) {}
        void calculate() const;
      private:
        Size timeSteps_;
    };


    // template definitions

    template <class T>
    void BinomialVanillaEngine<T>::calculate() const {

        double s0 = arguments_.blackScholesProcess->stateVariable()->value();
        double v = arguments_.blackScholesProcess->blackVolatility()->blackVol(
            arguments_.exercise->lastDate(), s0);
        Date maturityDate = arguments_.exercise->lastDate();
        Rate r = arguments_.blackScholesProcess->riskFreeRate()
            ->zeroYield(maturityDate);
        Rate q = arguments_.blackScholesProcess->dividendYield()
            ->zeroYield(maturityDate);
        Date referenceDate = 
            arguments_.blackScholesProcess->riskFreeRate()->referenceDate();
        Date todaysDate = 
            arguments_.blackScholesProcess->riskFreeRate()->todaysDate();
        DayCounter dc = 
            arguments_.blackScholesProcess->riskFreeRate()->dayCounter();

        // binomial trees with constant coefficient
        RelinkableHandle<TermStructure> flatRiskFree(
            boost::shared_ptr<TermStructure>(
                new FlatForward(todaysDate, referenceDate, r, dc)));
        RelinkableHandle<TermStructure> flatDividends(
            boost::shared_ptr<TermStructure>(
                new FlatForward(todaysDate, referenceDate, q, dc)));
        RelinkableHandle<BlackVolTermStructure> flatVol(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, v, dc)));

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Time maturity = arguments_.blackScholesProcess->riskFreeRate()
            ->dayCounter().yearFraction(referenceDate, maturityDate);

        boost::shared_ptr<DiffusionProcess> bs(
            new BlackScholesProcess(
                   RelinkableHandle<Quote>(
                             arguments_.blackScholesProcess->stateVariable()),
                   flatDividends, flatRiskFree, flatVol));
        boost::shared_ptr<Tree> tree(new T(bs, maturity, timeSteps_,
                                           payoff->strike()));

        boost::shared_ptr<Lattice> lattice(
            new BlackScholesLattice(tree, r, maturity, timeSteps_));

        boost::shared_ptr<DiscretizedAsset> option(
            new DiscretizedVanillaOption(lattice, arguments_));

        lattice->initialize(option, maturity);
        lattice->rollback(option, 0.0);
        results_.value = lattice->presentValue(option);
    }

}


#endif
