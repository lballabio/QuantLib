/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Cristina Duminuco
 Copyright (C) 2006, 2009 StatPro Italia srl
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>

namespace QuantLib {

    CappedFlooredCoupon::CappedFlooredCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate cap, Rate floor)
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
                         underlying->isInArrears()),
      underlying_(underlying),
      isCapped_(false), isFloored_(false) {

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
            QL_REQUIRE(cap >= floor,
                       "cap level (" << cap <<
                       ") less than floor level (" << floor << ")");
        }

        registerWith(underlying);
    }

    void CappedFlooredCoupon::setPricer(
                 const boost::shared_ptr<FloatingRateCouponPricer>& pricer) {
        FloatingRateCoupon::setPricer(pricer);
        underlying_->setPricer(pricer);
    }

    Rate CappedFlooredCoupon::rate() const {
        QL_REQUIRE(underlying_->pricer(), "pricer not set");
        Rate swapletRate = underlying_->rate();
        Rate floorletRate = 0.;
        if(isFloored_)
            floorletRate = underlying_->pricer()->floorletRate(effectiveFloor());
        Rate capletRate = 0.;
        if(isCapped_)
            capletRate = underlying_->pricer()->capletRate(effectiveCap());
        return swapletRate + floorletRate - capletRate;
    }

    Rate CappedFlooredCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

    Rate CappedFlooredCoupon::cap() const {
        if ( (gearing_ > 0) && isCapped_)
                return cap_;
        if ( (gearing_ < 0) && isFloored_)
            return floor_;
        return Null<Rate>();
    }

    Rate CappedFlooredCoupon::floor() const {
        if ( (gearing_ > 0) && isFloored_)
            return floor_;
        if ( (gearing_ < 0) && isCapped_)
            return cap_;
        return Null<Rate>();
    }

    Rate CappedFlooredCoupon::effectiveCap() const {
        if (isCapped_)
            return (cap_ - spread())/gearing();
        else
            return Null<Rate>();
    }

    Rate CappedFlooredCoupon::effectiveFloor() const {
        if (isFloored_)
            return (floor_ - spread())/gearing();
        else
            return Null<Rate>();
    }

    void CappedFlooredCoupon::update() {
        notifyObservers();
    }

    void CappedFlooredCoupon::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        Visitor<CappedFlooredCoupon>* v1 =

            dynamic_cast<Visitor<CappedFlooredCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

}
