/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file isdacdsengine.hpp
    \brief ISDA engine for credit default swaps
*/

#ifndef quantlib_isda_cds_engine_hpp
#define quantlib_isda_cds_engine_hpp

#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    /*! References:

        [1] The Pricing and Risk Management of Credit Default Swaps, with a
            Focus on the ISDA Model,
            OpenGamma Quantitative Research, Version as of 15-Oct-2013

        [2] ISDA CDS Standard Model Proposed Numerical Fix \ Thursday,
            November 15, 2012, Markit

        [3] Markit Interest Rate Curve XML Specifications,
            Version 1.16, Tuesday, 15 October 2013

    */

    class IsdaCdsEngine : public CreditDefaultSwap::engine {

      public:
        /*! According to [1] the settings for the flags
            AccrualBias / ForwardsInCouponPeriod corresponding
            to the standard model implementation C code are

            prior 1.8.2    HalfDayBias / Flat
            1.8.2          NoBias / Flat

            The theoretical correct setting would be NoBias / Piecewise

            Todo: Clarify in which version of the standard model
            implementation C code the numerical problem of zero denominators
            is solved and how exactly.
        */

        enum NumericalFix {
            None,  // as in [1] footnote 26 (i.e. 10^{-50} is added to
                   // denominators $f_i+h_i$$)
            Taylor // as in [2] i.e. for $f_i+h_i < 10^{-4}$ a Taylor expansion
                   // is used to avoid zero denominators
        };

        enum AccrualBias {
            HalfDayBias, // as in [1] formula (50), second (error) term is
                         // included
            NoBias // as in [1], but second term in formula (50) is not included
        };

        enum ForwardsInCouponPeriod {
            Flat, // as in [1], formula (52), second (error) term is included
            Piecewise // as in [1], but second term in formula (52) is not
                      // included
        };

        /*! Constructor where the client code is responsible for providing a
            default curve and an interest rate curve compliant with the ISDA
            specifications.

            To be precisely consistent with the ISDA specification
                bool IborCoupon::Settings::usingAtParCoupons();
            must be true. This is not checked in order not to
            kill the engine completely in this case.

            Furthermore, the ibor index in the swap rate helpers should not
            provide the evaluation date's fixing.
        */

        IsdaCdsEngine(Handle<DefaultProbabilityTermStructure> probability,
                      Real recoveryRate,
                      Handle<YieldTermStructure> discountCurve,
                      const ext::optional<bool>& includeSettlementDateFlows = ext::nullopt,
                      NumericalFix numericalFix = Taylor,
                      AccrualBias accrualBias = HalfDayBias,
                      ForwardsInCouponPeriod forwardsInCouponPeriod = Piecewise);

        Handle<YieldTermStructure> isdaRateCurve() const { return discountCurve_; }
        Handle<DefaultProbabilityTermStructure> isdaCreditCurve() const { return probability_; }

        void calculate() const override;

      private:
        Handle<DefaultProbabilityTermStructure> probability_;
        const Real recoveryRate_;
        Handle<YieldTermStructure> discountCurve_;
        const ext::optional<bool> includeSettlementDateFlows_;
        const NumericalFix numericalFix_;
        const AccrualBias accrualBias_;
        const ForwardsInCouponPeriod forwardsInCouponPeriod_;
    };
}

#endif
