/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

#include <ql/cashflows/equitynotionalresetcoupon.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/patterns/visitor.hpp>
#include <utility>

namespace QuantLib {

    EquityNotionalResetCoupon::EquityNotionalResetCoupon(
        const ext::shared_ptr<FloatingRateCoupon>& underlying,
        ext::shared_ptr<EquityIndex> equityIndex,
        const Date& initialDate,
        const Date& resetDate)
    : FloatingRateCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->index(),
                         underlying->gearing(),
                         underlying->spread(),
                         underlying->referencePeriodStart(),
                         underlying->referencePeriodEnd(),
                         underlying->dayCounter(),
                         underlying->isInArrears(),
                         underlying->exCouponDate(),
                         underlying->fixingConvention()),
      underlying_(underlying), equityIndex_(std::move(equityIndex)), initialDate_(initialDate),
      resetDate_(resetDate) {
        QL_REQUIRE(equityIndex_, "no equity index provided");
        QL_REQUIRE(initialDate_ <= resetDate_,
                   "initial date (" << initialDate_ << ") cannot be later than reset date ("
                                    << resetDate_ << ")");
        // the amount is not cached, so notifications can always be forwarded
        alwaysForwardNotifications();
        registerWith(underlying_);
        registerWith(equityIndex_);
    }

    Real EquityNotionalResetCoupon::nominalScale() const {
        return equityIndex_->fixing(resetDate_) / equityIndex_->fixing(initialDate_);
    }

    Real EquityNotionalResetCoupon::nominal() const {
        return underlying_->nominal() * nominalScale();
    }

    Real EquityNotionalResetCoupon::amount() const {
        return underlying_->amount() * nominalScale();
    }

    Real EquityNotionalResetCoupon::accruedAmount(const Date& d) const {
        return underlying_->accruedAmount(d) * nominalScale();
    }

    void EquityNotionalResetCoupon::setPricer(
        const ext::shared_ptr<FloatingRateCouponPricer>& pricer) {
        underlying_->setPricer(pricer);
        FloatingRateCoupon::setPricer(pricer);
    }

    void EquityNotionalResetCoupon::deepUpdate() {
        update();
        underlying_->deepUpdate();
    }

    void EquityNotionalResetCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<EquityNotionalResetCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

}
