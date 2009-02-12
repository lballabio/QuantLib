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

#include <ql/experimental/inflation/capflooredinflationcoupon.hpp>
#include <ql/experimental/inflation/inflationcouponpricer.hpp>

namespace QuantLib {

    void CappedFlooredYoYInflationCoupon::setCommon(
                                         const Rate &cap, const Rate &floor) {

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
                      const boost::shared_ptr<YoYInflationCoupon>& underlying,
                      Rate cap, Rate floor, bool paysWithin)
    : YoYInflationCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->lag(),
                         underlying->fixingDays(),
                         underlying->index(),
                         underlying->gearing(),
                         underlying->spread(),
                         underlying->dayCounter(),
                         underlying->businessDayConvention(),
                         underlying->calendar(),
                         underlying->referencePeriodStart(),
                         underlying->referencePeriodEnd()),
      underlying_(underlying),
      paysWithin_(paysWithin), isFloored_(false), isCapped_(false) {

        setCommon(cap, floor);
        registerWith(underlying);
    }


    void CappedFlooredYoYInflationCoupon::setPricer(
                 const boost::shared_ptr<YoYInflationCouponPricer>& pricer) {
        if(pricer_)
            unregisterWith(pricer_);
        pricer_ = pricer;
        QL_REQUIRE(pricer_, "no adequate pricer given");
        registerWith(pricer_);
        update();
        underlying_->setPricer(pricer);
    }


    Rate CappedFlooredYoYInflationCoupon::rate() const {
        QL_REQUIRE(underlying_->pricer(), "pricer not set");
        Rate swapletRate = underlying_->rate();
        Rate floorletRate = 0.;
        if(isFloored_)
            floorletRate =
                underlying_->pricer()->floorletRate(effectiveFloor());
        Rate capletRate = 0.;
        if(isCapped_)
            capletRate = underlying_->pricer()->capletRate(effectiveCap());

        if (paysWithin_) return swapletRate + floorletRate - capletRate;
        else  return capletRate + floorletRate;
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

}

