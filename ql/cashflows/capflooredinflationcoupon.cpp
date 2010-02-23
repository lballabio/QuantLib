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
                const boost::shared_ptr<YoYInflationCoupon>& underlying,
                        Rate cap, Rate floor)
    : YoYInflationCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->yoyIndex(),
                         underlying->observationLag(),
                         underlying->dayCounter(),
                         underlying->gearing(),
                         underlying->spread(),
                         underlying->referencePeriodStart(),
                         underlying->referencePeriodEnd()),
      underlying_(underlying), isFloored_(false), isCapped_(false) {

        setCommon(cap, floor);
        registerWith(underlying);
    }


    void CappedFlooredYoYInflationCoupon::setPricer(
            const boost::shared_ptr<YoYInflationCouponPricer>& pricer) {

        YoYInflationCoupon::setPricer(pricer);
        if (underlying_) underlying_->setPricer(pricer);
    }


    Rate CappedFlooredYoYInflationCoupon::rate() const {
        Rate swapletRate = underlying_ ? underlying_->rate() : YoYInflationCoupon::rate();

        if(isFloored_ || isCapped_) {
            if (underlying_) {
                QL_REQUIRE(underlying_->pricer(), "pricer not set");
            } else {
                QL_REQUIRE(pricer_, "pricer not set");
            }
        }

        Rate floorletRate = 0.;
        if(isFloored_) {
            floorletRate =
            underlying_ ?
            underlying_->pricer()->floorletRate(effectiveFloor()) :
            pricer()->floorletRate(effectiveFloor())
            ;
        }
        Rate capletRate = 0.;
        if(isCapped_) {
            capletRate =
            underlying_ ?
            underlying_->pricer()->capletRate(effectiveCap()) :
            pricer()->capletRate(effectiveCap())
            ;
        }

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
        Visitor<CappedFlooredYoYInflationCoupon>* v1 =
            dynamic_cast<Visitor<CappedFlooredYoYInflationCoupon>*>(&v);

        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

}

