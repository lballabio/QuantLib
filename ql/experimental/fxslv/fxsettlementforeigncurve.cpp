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

#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    namespace detail {

        Date dayContaining(const TermStructure& ts, Time t) {
            // guess from an Actual/365 count, then step to the last day
            // starting at or before t, whatever the day counter
            const Date referenceDay(ts.referenceDate().serialNumber());
            const Real eps = 1.0e-12;
            Date d = referenceDay +
                     static_cast<Date::serial_type>(std::floor(std::max(t, 0.0) * 365.0));
            while (d > referenceDay && ts.timeFromReference(d) > t + eps)
                --d;
            while (ts.timeFromReference(d + 1) <= t + eps)
                ++d;
            return d;
        }

    }

    FxSettlementForeignCurve::FxSettlementForeignCurve(Handle<YieldTermStructure> foreignCurve,
                                                       Handle<YieldTermStructure> domesticCurve,
                                                       FxSettleConvention settlement)
    : RelativeDerivedYieldTermStructure<YieldTermStructure>(std::move(domesticCurve)),
      foreignCurve_(std::move(foreignCurve)), settlement_(std::move(settlement)) {
        registerWith(foreignCurve_);
    }

    Real FxSettlementForeignCurve::forwardOverSpot(Time t) const {
        const Date spotDate = settlement_.valueDate(referenceDate());
        const Date deliveryDate = settlement_.valueDate(detail::dayContaining(*this, t));
        const Handle<YieldTermStructure>& domestic = originalCurve_;
        return (foreignCurve_->discount(deliveryDate, true) / foreignCurve_->discount(spotDate, true)) /
               (domestic->discount(deliveryDate, true) / domestic->discount(spotDate, true));
    }

    DiscountFactor FxSettlementForeignCurve::discountImpl(Time t) const {
        // P_f(t) such that S P_f(t) / P_d(t) is the settlement forward
        return originalCurve_->discount(t, true) * forwardOverSpot(t);
    }

}
