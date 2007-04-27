/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti


 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/digitalcoupon.hpp>

namespace QuantLib {

    DigitalCoupon::DigitalCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate callStrike,
                  Rate putStrike,
                  Rate cashRate,
                  Real eps)
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
      hasLowerStrike_(false), hasUpperStrike_(false),
      eps_(eps), isCashOrNothing_(false) {

        if (putStrike != Null<Rate>() && callStrike != Null<Rate>())
            QL_REQUIRE(putStrike >= callStrike, "put strike (" << putStrike <<
                                            ") < call strike " << callStrike);
        if (callStrike != Null<Rate>()){
            QL_REQUIRE(callStrike >= 0., "negative call strike not allowed");
            hasLowerStrike_ = true;
            lowerStrike_ = (callStrike - underlying->spread())/underlying->gearing();
        }
        if (putStrike != Null<Rate>()){
            QL_REQUIRE(putStrike >= 0., "negative put strike not allowed");
            hasUpperStrike_ = true;
            upperStrike_ = (putStrike - underlying->spread())/underlying->gearing();
        }
        if (cashRate != Null<Rate>()){
            cashRate_ = cashRate;
            isCashOrNothing_ = true;
        }
        registerWith(underlying);
    }

    Rate DigitalCoupon::optionRate() const {

        Rate callOptionRate = isCashOrNothing_ ? cashRate_ : underlying_->rate();

        // Call digital option
        if(hasLowerStrike_&& lowerStrike_>eps_) {
            boost::shared_ptr<CappedFlooredCoupon> next(
                new CappedFlooredCoupon(underlying_, lowerStrike_ + eps_, Null<Rate>()));
            boost::shared_ptr<CappedFlooredCoupon> previous(
                new CappedFlooredCoupon(underlying_, lowerStrike_ - eps_, Null<Rate>()));
            if(isCashOrNothing_) {
                callOptionRate = cashRate_ * 
                                (next->rate() - previous->rate()) / (2*eps_);
            } else {
                // if asset-or-nothing
                callOptionRate = lowerStrike_ * 
                                 (next->rate() - previous->rate()) / (2*eps_);
                callOptionRate += underlying_->rate() - next->rate();
            }
        }

        /* Floor digital option: 
           the putOptionRate is calculated as a call digital option
           to subract from the callOptionRate */
        Rate putOptionRate = 0.;
        if(hasUpperStrike_){
            if(upperStrike_ < eps_){
                putOptionRate = cashRate_;
            } else {
                boost::shared_ptr<CappedFlooredCoupon> next(
                    new CappedFlooredCoupon(underlying_, upperStrike_ + eps_, Null<Rate>()));
                boost::shared_ptr<CappedFlooredCoupon> previous(
                    new CappedFlooredCoupon(underlying_, upperStrike_ - eps_, Null<Rate>()));
                if(isCashOrNothing_){
                    putOptionRate = cashRate_ * 
                                   (next->rate() - previous->rate()) / (2*eps_);
                } else {
                    // if asset-or-nothing
                    putOptionRate = upperStrike_ * 
                                    (next->rate() - previous->rate()) / (2*eps_);
                    putOptionRate += underlying_->rate() - next->rate();
                }
            }
        }

        return callOptionRate - putOptionRate;
    }

    Rate DigitalCoupon::rate() const {
      
        return underlying_->rate() - optionRate();
    }

    Rate DigitalCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

    Rate DigitalCoupon::callStrike() const {
            //if(hasLowerStrike_)
                return lowerStrike_;
            //else
            //    return NuRate(0.);
   }

    Rate DigitalCoupon::putStrike() const {
            //if(hasUpperStrike_)
                return upperStrike_;
            //else
            //    return Rate(1000.);
    }

    void DigitalCoupon::update() {
        notifyObservers();
    }

    void DigitalCoupon::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        Visitor<DigitalCoupon>* v1 =

            dynamic_cast<Visitor<DigitalCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

}
