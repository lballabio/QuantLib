
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file vanillaengines.hpp
    \brief Vanilla option engines
*/

#ifndef quantlib_vanilla_engines_h
#define quantlib_vanilla_engines_h

#include <ql/Instruments/vanillaoption.hpp>
#include <ql/Lattices/binomialtree.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/PricingEngines/Vanilla/discretizedvanillaoption.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    //! Vanilla option engine base class
    class VanillaEngine : public GenericEngine<VanillaOption::arguments,
                                               VanillaOption::results> {};

    //! Pricing engine for European vanilla options using analytical formulae
    class AnalyticEuropeanEngine : public VanillaEngine {
      public:
        void calculate() const;
    };

    /*! Pricing engine for American vanilla options with digital payoff
        using analytic formulae

        \todo add more greeks (as of now only delta and rho available)
    */
    class AnalyticAmericanEngine : public VanillaEngine {
      public:
        void calculate() const;
    };

    /*! Pricing engine for American vanilla options with 
        Barone-Adesi and Whaley approximation (1987)
    */
    class BaroneAdesiWhaleyApproximationEngine : public VanillaEngine {
      public:
        void calculate() const;
    };

    /*! Pricing engine for American vanilla options with 
        Bjerksund and Stensland approximation (1993)
    */
    class BjerksundStenslandApproximationEngine : public VanillaEngine {
      public:
        void calculate() const;
    };

    //! Pricing engine for European vanilla options using integral approach
    class IntegralEngine : public VanillaEngine {
      public:
        void calculate() const;
    };


    //! Pricing engine for vanilla options using binomial trees
    template <class TreeType>
    class BinomialVanillaEngine : public VanillaEngine {
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

        double s0 = arguments_.blackScholesProcess->stateVariable->value();
        double v = arguments_.blackScholesProcess->volTS->blackVol(
            arguments_.exercise->lastDate(), s0);
        Date maturityDate = arguments_.exercise->lastDate();
        Rate r = arguments_.blackScholesProcess->riskFreeTS->zeroYield(maturityDate);
        Rate q = arguments_.blackScholesProcess->dividendTS->zeroYield(maturityDate);
        Date referenceDate = arguments_.blackScholesProcess->riskFreeTS->referenceDate();
        Date todaysDate    = arguments_.blackScholesProcess->riskFreeTS->todaysDate();
        DayCounter dc      = arguments_.blackScholesProcess->riskFreeTS->dayCounter();

        // binomial trees with constant coefficient
        RelinkableHandle<TermStructure> flatRiskFree(
            Handle<TermStructure>(
                new FlatForward(todaysDate, referenceDate, r, dc)));
        RelinkableHandle<TermStructure> flatDividends(
            Handle<TermStructure>(
                new FlatForward(todaysDate, referenceDate, q, dc)));
        RelinkableHandle<BlackVolTermStructure> flatVol(
            Handle<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, v, dc)));

        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticEuropeanEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif

        Time maturity = arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
            referenceDate, maturityDate);;

        Handle<DiffusionProcess> bs(
            new BlackScholesProcess(flatRiskFree, flatDividends, flatVol,s0));
        Handle<Tree> tree(new T(bs, maturity, timeSteps_,
                                payoff->strike()));

        Handle<Lattice> lattice(
            new BlackScholesLattice(tree, r, maturity, timeSteps_));

        Handle<DiscretizedAsset> option(
            new DiscretizedVanillaOption(lattice, arguments_));

        lattice->initialize(option, maturity);
        lattice->rollback(option, 0.0);
        results_.value = lattice->presentValue(option);
    }

}


#endif
