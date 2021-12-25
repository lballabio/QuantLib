/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/experimental/coupons/strippedcapflooredcoupon.hpp>
#include <utility>

namespace QuantLib {

    StrippedCappedFlooredCoupon::StrippedCappedFlooredCoupon(
        const ext::shared_ptr<CappedFlooredCoupon> &underlying)
        : FloatingRateCoupon(
              underlying->date(), underlying->nominal(),
              underlying->accrualStartDate(), underlying->accrualEndDate(),
              underlying->fixingDays(), underlying->index(),
              underlying->gearing(), underlying->spread(),
              underlying->referencePeriodStart(),
              underlying->referencePeriodEnd(), underlying->dayCounter(),
              underlying->isInArrears()),
          underlying_(underlying) {
        registerWith(underlying);
    }

    Rate StrippedCappedFlooredCoupon::rate() const {

        QL_REQUIRE(underlying_->underlying()->pricer() != nullptr, "pricer not set");
        underlying_->underlying()->pricer()->initialize(*underlying_->underlying());
        Rate floorletRate = 0.0;
        if (underlying_->isFloored())
            floorletRate = underlying_->underlying()->pricer()->floorletRate(
                underlying_->effectiveFloor());
        Rate capletRate = 0.0;
        if (underlying_->isCapped())
            capletRate =
                underlying_->underlying()->pricer()->capletRate(underlying_->effectiveCap());

        // if the underlying is collared we return the value of the embedded
        // collar, otherwise the value of a long floor or a long cap respectively

        return (underlying_->isFloored() && underlying_->isCapped())
                   ? floorletRate - capletRate
                   : floorletRate + capletRate;
    }

    Rate StrippedCappedFlooredCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

    Rate StrippedCappedFlooredCoupon::cap() const { return underlying_->cap(); }

    Rate StrippedCappedFlooredCoupon::floor() const {
        return underlying_->floor();
    }

    Rate StrippedCappedFlooredCoupon::effectiveCap() const {
        return underlying_->effectiveCap();
    }

    Rate StrippedCappedFlooredCoupon::effectiveFloor() const {
        return underlying_->effectiveFloor();
    }

    void StrippedCappedFlooredCoupon::update() { notifyObservers(); }

    void StrippedCappedFlooredCoupon::accept(AcyclicVisitor &v) {
        underlying_->accept(v);
        auto* v1 = dynamic_cast<Visitor<StrippedCappedFlooredCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    bool StrippedCappedFlooredCoupon::isCap() const {
        return underlying_->isCapped();
    }

    bool StrippedCappedFlooredCoupon::isFloor() const {
        return underlying_->isFloored();
    }

    bool StrippedCappedFlooredCoupon::isCollar() const {
        return isCap() && isFloor();
    }

    void StrippedCappedFlooredCoupon::setPricer(
        const ext::shared_ptr<FloatingRateCouponPricer> &pricer) {
        FloatingRateCoupon::setPricer(pricer);
        underlying_->setPricer(pricer);
    }

    StrippedCappedFlooredCouponLeg::StrippedCappedFlooredCouponLeg(Leg underlyingLeg)
    : underlyingLeg_(std::move(underlyingLeg)) {}

    StrippedCappedFlooredCouponLeg::operator Leg() const {
        Leg resultLeg;
        resultLeg.reserve(underlyingLeg_.size());
        ext::shared_ptr<CappedFlooredCoupon> c;
        for (const auto& i : underlyingLeg_) {
            if ((c = ext::dynamic_pointer_cast<CappedFlooredCoupon>(i)) != nullptr) {
                resultLeg.push_back(
                    ext::make_shared<StrippedCappedFlooredCoupon>(c));
            } else {
                resultLeg.push_back(i);
            }
        }
        return resultLeg;
    }
}
