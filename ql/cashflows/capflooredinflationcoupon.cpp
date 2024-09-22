/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/cashflows/capflooredinflationcoupon.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>

namespace QuantLib {

    void CappedFlooredYoYInflationCoupon::setCommon(
        Rate cap, Rate floor) {

        isCapped_ = false;
        isFloored_ = false;

        if (gearing_ > 0) {
            if (cap != Null<Rate>()) {
                isCapped_ = true;
                cap_ = cap;
            }
            if (floor != Null<Rate>()) {
                floor_ = floor;
                isFloored_ = true;
            }
        } else {
            if (cap != Null<Rate>()){
                floor_ = cap;
                isFloored_ = true;
            }
            if (floor != Null<Rate>()){
                isCapped_ = true;
                cap_ = floor;
            }
        }
        if (isCapped_ && isFloored_) {
            QL_REQUIRE(cap >= floor, "cap level (" << cap <<
                       ") less than floor level (" << floor << ")");
        }
    }


    CappedFlooredYoYInflationCoupon::CappedFlooredYoYInflationCoupon(
                const ext::shared_ptr<YoYInflationCoupon>& underlying,
                        Rate cap, Rate floor)
    : YoYInflationCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->yoyIndex(),
                         underlying->observationLag(),
                         underlying->interpolation(),
                         underlying->dayCounter(),
                         underlying->gearing(),
                         underlying->spread(),
                         underlying->referencePeriodStart(),
                         underlying->referencePeriodEnd()),
      underlying_(underlying), isFloored_(false), isCapped_(false) {
        CappedFlooredYoYInflationCoupon::setCommon(cap, floor);
        registerWith(underlying);
    }


    void CappedFlooredYoYInflationCoupon::setPricer(
            const ext::shared_ptr<YoYInflationCouponPricer>& pricer) {

        YoYInflationCoupon::setPricer(pricer);
        if (underlying_ != nullptr)
            underlying_->setPricer(pricer);
    }


    Rate CappedFlooredYoYInflationCoupon::underlyingRate() const {
        return underlying_ != nullptr ? underlying_->rate() : YoYInflationCoupon::rate();
    }

    Rate CappedFlooredYoYInflationCoupon::rate() const {
        Rate swapletRate = underlyingRate();

        auto couponPricer = underlying_ != nullptr ? underlying_->pricer() : pricer();

        if (isFloored_ || isCapped_) {
            QL_REQUIRE(couponPricer, "pricer not set");
        }

        Rate floorletRate = isFloored_ ? couponPricer->floorletRate(effectiveFloor()) : 0.0;
        Rate capletRate = isCapped_? couponPricer->capletRate(effectiveCap()) : 0.0;

        return swapletRate + floorletRate - capletRate;
    }


    Rate CappedFlooredYoYInflationCoupon::cap() const {
        if ( (gearing_ > 0) && isCapped_)
            return cap_;
        if ( (gearing_ < 0) && isFloored_)
            return floor_;
        return Null<Rate>();
    }


    Rate CappedFlooredYoYInflationCoupon::floor() const {
        if ( (gearing_ > 0) && isFloored_)
            return floor_;
        if ( (gearing_ < 0) && isCapped_)
            return cap_;
        return Null<Rate>();
    }


    Rate CappedFlooredYoYInflationCoupon::effectiveCap() const {
        return  (cap_ - spread())/gearing();
    }


    Rate CappedFlooredYoYInflationCoupon::effectiveFloor() const {
        return  (floor_ - spread())/gearing();
    }


    void CappedFlooredYoYInflationCoupon::update() {
        notifyObservers();
    }


    void CappedFlooredYoYInflationCoupon::accept(AcyclicVisitor& v) {
        typedef YoYInflationCoupon super;
        auto* v1 = dynamic_cast<Visitor<CappedFlooredYoYInflationCoupon>*>(&v);

        if (v1 != nullptr)
            v1->visit(*this);
        else
            super::accept(v);
    }

}

