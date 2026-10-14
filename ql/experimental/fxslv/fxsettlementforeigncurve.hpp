/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file fxsettlementforeigncurve.hpp
    \brief Foreign curve giving a Black-Scholes process the FX settlement forward
*/

#ifndef quantlib_fx_settlement_foreign_curve_hpp
#define quantlib_fx_settlement_foreign_curve_hpp

#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/termstructures/yield/derivedtermstructure.hpp>

namespace QuantLib {

    namespace detail {

        //! The day, at midnight, containing the given time from a term structure's reference date.
        Date dayContaining(const TermStructure& ts, Time t);

    }

    //! Foreign curve giving a Black-Scholes process the FX settlement forward
    /*! A Black-Scholes process takes its forward to a time \f$ t \f$ as
        \f$ S\,P_f(t)/P_d(t) \f$, from today to the expiry.  An FX option
        expiring at \f$ t \f$ delivers on the delivery date of its expiry
        day, and its forward runs from the spot date to that delivery
        date:
        \f[
            F(t) = S\,\frac{P_f(s_0, T_d)}{P_d(s_0, T_d)}.
        \f]
        The two differ by the carry over the difference between the spot
        and delivery lags, which a weekend or holiday in one lag and not
        the other makes non-zero.  Passed to the process as its foreign
        (dividend) curve with the domestic curve as its risk-free curve,
        this curve makes the process forward \f$ F(t) \f$, so that the
        process, Dupire local volatility and an FX variance surface all
        use the same forward.

        It shares the domestic curve's reference date, day counter and
        time axis.  \f$ F(t) \f$ is constant within each expiry day and
        steps at midnight, as the delivery date rolls.

        makeFxLocalVolProcess() builds a process with this curve and a
        consistent local vol, and is the preferred way to use it.
    */
    class FxSettlementForeignCurve
    : public RelativeDerivedYieldTermStructure<YieldTermStructure> {
      public:
        FxSettlementForeignCurve(Handle<YieldTermStructure> foreignCurve,
                                 Handle<YieldTermStructure> domesticCurve,
                                 FxSettleConvention settlement);

        //! Forward for an option expiring at the given time, over spot.
        Real forwardOverSpot(Time t) const;

      protected:
        DiscountFactor discountImpl(Time t) const override;

      private:
        Handle<YieldTermStructure> foreignCurve_;
        FxSettleConvention settlement_;
    };

}

#endif
