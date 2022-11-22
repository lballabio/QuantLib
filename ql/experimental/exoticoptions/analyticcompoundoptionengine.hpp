/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dimitri Reiswich

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

/*! \file analyticcompoundoptionengine.hpp
    \brief Analytic compound option engines
*/

#ifndef quantlib_analytic_compound_option_engine_hpp
#define quantlib_analytic_compound_option_engine_hpp

#include <ql/experimental/exoticoptions/compoundoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for compound options using analytical formulae
    /*! The formulas are taken from "Foreign Exchange Risk",
        Uwe Wystup, Risk 2002, where closed form Greeks are available.
        (not available in Haug 2007).
        Value: Page 84, Greeks: Pages 94-95.

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class AnalyticCompoundOptionEngine : public CompoundOption::engine {
      public:
        explicit AnalyticCompoundOptionEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        CumulativeNormalDistribution N_;
        NormalDistribution n_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;

        // helper methods
        Time residualTimeMother() const;
        Time residualTimeDaughter() const;
        Time residualTimeMotherDaughter() const;

        Date maturityMother() const;
        Date maturityDaughter() const;

        Real dPlus() const;
        Real dMinus() const;

        Real dPlusTau12(Real S) const;
        Real dMinusTau12() const;

        Real strikeDaughter() const;
        Real strikeMother() const;

        Real spot() const;

        Real volatilityDaughter() const;
        Real volatilityMother() const;

        Real riskFreeRateDaughter() const;
        Real dividendRateDaughter() const;

        Real stdDeviationDaughter() const;
        Real stdDeviationMother() const;

        Real typeDaughter() const;
        Real typeMother() const;

        Real transformX(Real X) const;
        Real e(Real X) const;

        DiscountFactor riskFreeDiscountDaughter() const;
        DiscountFactor riskFreeDiscountMother() const;
        DiscountFactor riskFreeDiscountMotherDaughter() const;

        DiscountFactor dividendDiscountDaughter() const;
        DiscountFactor dividendDiscountMother() const;
        DiscountFactor dividendDiscountMotherDaughter() const;

        ext::shared_ptr<PlainVanillaPayoff> payoffMother() const;
        ext::shared_ptr<PlainVanillaPayoff> payoffDaughter() const;
    };

}

#endif
