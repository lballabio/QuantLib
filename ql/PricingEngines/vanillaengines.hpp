
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
#include <ql/PricingEngines/genericengine.hpp>
#include <ql/Lattices/binomialtree.hpp>
#include <ql/PricingEngines/discretizedvanillaoption.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    //! Vanilla engine base class
    class VanillaEngine : public GenericEngine<VanillaOption::arguments,
                                               VanillaOption::results> {};

    //! Pricing engine for European options using analytical formulae
    class AnalyticEuropeanEngine : public VanillaEngine {
      public:
        void calculate() const;
      private:
        #if defined(QL_PATCH_SOLARIS)
        CumulativeNormalDistribution f_;
        #else
        static const CumulativeNormalDistribution f_;
        #endif
    };


    //! Pricing engine for Vanilla options using integral approach
    class IntegralEngine : public VanillaEngine {
      public:
        void calculate() const;
    };


    //! Pricing engine for Vanilla options using binomial trees
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

        double s0 = arguments_.underlying;
        double v = arguments_.volTS->blackVol(arguments_.maturity, s0);
        Rate r = arguments_.riskFreeTS->zeroYield(arguments_.maturity);
        Rate q = arguments_.dividendTS->zeroYield(arguments_.maturity);
        Date referenceDate = arguments_.riskFreeTS->referenceDate();
        Date todaysDate    = arguments_.riskFreeTS->todaysDate();
        DayCounter dc      = arguments_.riskFreeTS->dayCounter();

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

        Handle<DiffusionProcess> bs(
            new BlackScholesProcess(flatRiskFree, flatDividends, flatVol,s0));
        Handle<Tree> tree(new T(bs, arguments_.maturity, timeSteps_));

        Handle<Lattice> lattice(
            new BlackScholesLattice(tree, r, arguments_.maturity, timeSteps_));

        Handle<DiscretizedAsset> option(
            new DiscretizedVanillaOption(lattice,arguments_));

        lattice->initialize(option, arguments_.maturity);
        lattice->rollback(option, 0.0);
        results_.value = lattice->presentValue(option);
    }

}


#endif
