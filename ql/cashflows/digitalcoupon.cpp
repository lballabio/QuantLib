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

    DigitalCoupon::DigitalCoupon(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                                 Rate callStrike,
                                 Rate putStrike,
                                 Rate cashRate,
                                 bool isCallOptionAdded,
                                 bool isPutOptionAdded,
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
      hasCallStrike_(false), hasPutStrike_(false),
      eps_(eps), isCashOrNothing_(false) {

        QL_REQUIRE(gearing()==1.0 && (spread()==Null<Rate>() || spread()==0.),
                   "Gearing must be equal to 1 and spread must be null. Temporary requirement");
        
        QL_REQUIRE(eps>0.0, "Non positive epsilon not allowed");

        if (putStrike != Null<Rate>() && callStrike != Null<Rate>()) {
            QL_REQUIRE(putStrike >= callStrike,
                       "put strike (" << putStrike << ") < " <<
                       "call strike (" << callStrike << ")");
            QL_REQUIRE(isCallOptionAdded == isPutOptionAdded,
                       "both call and put options must be added or subtracted");
        }

        if (callStrike != Null<Rate>()){
            QL_REQUIRE(callStrike >= 0., "negative call strike not allowed");
            hasCallStrike_ = true;
            callStrike_ = callStrike;
            effectiveCallStrike_ = (callStrike - spread())/gearing();
            QL_REQUIRE(effectiveCallStrike_>=eps_, "call strike < eps");
            callCsi_ = isCallOptionAdded ? 1.0 : -1.0;
        }
        if (putStrike != Null<Rate>()){
            QL_REQUIRE(putStrike >= 0., "negative put strike not allowed");
            hasPutStrike_ = true;
            putStrike_ = putStrike;
            effectivePutStrike_ = (putStrike - spread())/gearing();
            putCsi_ = isPutOptionAdded ? 1.0 : -1.0;
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
        if(hasCallStrike_) {
            boost::shared_ptr<CappedFlooredCoupon> next(
                new CappedFlooredCoupon(underlying_, effectiveCallStrike_ + eps_));
            boost::shared_ptr<CappedFlooredCoupon> previous(
                new CappedFlooredCoupon(underlying_, effectiveCallStrike_ - eps_));
            callOptionRate *= (next->rate() - previous->rate()) / (2.*eps_);
        }

        /* Put digital option: 
           the putOptionRate is calculated as a call digital option
           to subract from the callOptionRate */
        Rate putOptionRate = 0.;
        if(hasPutStrike_){
            if(putStrike_ < eps_){
                putOptionRate = cashRate_;
            } else {
                boost::shared_ptr<CappedFlooredCoupon> next(
                    new CappedFlooredCoupon(underlying_, effectivePutStrike_ + eps_));
                boost::shared_ptr<CappedFlooredCoupon> previous(
                    new CappedFlooredCoupon(underlying_, effectivePutStrike_ - eps_));
                if(isCashOrNothing_){
                    putOptionRate = cashRate_ *
                                   (next->rate() - previous->rate()) / (2.*eps_);
                } else {
                    // if asset-or-nothing
                    putOptionRate = underlying_->rate() *
                                    (next->rate() - previous->rate()) / (2.*eps_);
                }
            }
        }

        return callOptionRate - putOptionRate;
    }

    Rate DigitalCoupon::rate() const {
        QL_REQUIRE(underlying_->pricer(), "pricer not set");
        Real csi = isCall() ? callCsi_ : putCsi_;
        return underlying_->rate() + csi * optionRate();
    }

    Rate DigitalCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

    Rate DigitalCoupon::callStrike() const {
        return callStrike_;
   }

    Rate DigitalCoupon::putStrike() const {
        return putStrike_;
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

    void DigitalIborCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        Visitor<DigitalIborCoupon>* v1 =
            dynamic_cast<Visitor<DigitalIborCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

    void DigitalCmsCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        Visitor<DigitalCmsCoupon>* v1 =
            dynamic_cast<Visitor<DigitalCmsCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }



}

