
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
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              checking it against analytic results.
    */
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

        const boost::shared_ptr<BlackScholesProcess>& process =
            this->arguments_.blackScholesProcess;

        Real s0 = process->stateVariable()->value();
        Volatility v = process->blackVolatility()->blackVol(
            arguments_.exercise->lastDate(), s0);
        Date maturityDate = arguments_.exercise->lastDate();
        Rate r = process->riskFreeRate()->zeroYield(maturityDate);
        Rate q = process->dividendYield()->zeroYield(maturityDate);
        Date referenceDate = process->riskFreeRate()->referenceDate();
        #ifndef QL_DISABLE_DEPRECATED
        DayCounter rfdc = process->riskFreeRate()->dayCounter();
        #else
        DayCounter rfdc = Settings::instance().dayCounter();
        #endif

        // binomial trees with constant coefficient
        Handle<YieldTermStructure> flatRiskFree(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, r, rfdc)));
        Handle<YieldTermStructure> flatDividends(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, q, rfdc)));
        Handle<BlackVolTermStructure> flatVol(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, v, rfdc)));

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Time maturity = rfdc.yearFraction(referenceDate, maturityDate);

        boost::shared_ptr<StochasticProcess> bs(new
            BlackScholesProcess(Handle<Quote>(process->stateVariable()),
                                flatDividends, flatRiskFree, flatVol));
        boost::shared_ptr<Tree> tree(new T(bs, maturity, timeSteps_,
                                           payoff->strike()));

        boost::shared_ptr<Lattice> lattice(
            new BlackScholesLattice(tree, r, maturity, timeSteps_));

        DiscretizedVanillaOption option(arguments_);

        option.initialize(lattice, maturity);
        option.rollback(0.0);
        results_.value = option.presentValue();
    }

}


#endif
