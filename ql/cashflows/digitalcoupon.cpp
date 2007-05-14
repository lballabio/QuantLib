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
                                 bool longCallOption,
                                 Rate putStrike,
                                 bool longPutOption,
                                 Rate digitalPayoff,                              
                                 Replication::Type replication,
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
      underlying_(underlying), callCsi_(Rate(0.)), putCsi_(Rate(0.)),
      hasCallStrike_(false), hasPutStrike_(false),
      eps_(eps), putLeftEps_(eps_/2.), putRightEps_(eps_/2.),
      callLeftEps_(eps_/2.), callRightEps_(eps_/2.),
      isCashOrNothing_(false), replicationType_(replication) {
      
        QL_REQUIRE(eps>0.0, "Non positive epsilon not allowed");

        if (putStrike == Null<Rate>() && callStrike == Null<Rate>())
            QL_REQUIRE(digitalPayoff == Null<Rate>(), "Cash rate non allowed if call-put strike are both null");

        if (putStrike != Null<Rate>() && callStrike != Null<Rate>()) {
            QL_REQUIRE(putStrike >= callStrike,
                       "put strike (" << putStrike << ") < " <<
                       "call strike (" << callStrike << ")");
        }

        if (callStrike != Null<Rate>()){
            QL_REQUIRE(callStrike >= 0., "negative call strike not allowed");
            hasCallStrike_ = true;
            callStrike_ = callStrike;
            QL_REQUIRE(callStrike_>=eps_/2., "call strike < eps/2");
            callCsi_ = longCallOption ? 1.0 : -1.0;
        }
        if (putStrike != Null<Rate>()){
            QL_REQUIRE(putStrike >= 0., "negative put strike not allowed");
            hasPutStrike_ = true;
            putStrike_ = putStrike;
            putCsi_ = longPutOption ? 1.0 : -1.0;
        }
        if (digitalPayoff != Null<Rate>()){
            digitalPayoff_ = digitalPayoff;
            isCashOrNothing_ = true;
        }

        switch (replication) {
          case Replication::Central :
            // do nothing
            break;
          case Replication::Sub :
            if (hasCallStrike_) {
                if(longCallOption) {
                    callLeftEps_ = 0.;
                    callRightEps_ = eps_;
                } else {
                    callLeftEps_ = eps_;
                    callRightEps_ = 0.;
                }
            }
            if (hasPutStrike_) {
                if(longPutOption) {
                    putLeftEps_ = eps_;
                    putRightEps_ = 0.;
                } else {
                    putLeftEps_ = 0.;
                    putRightEps_ = eps_;
                }
            }
            break;
          case Replication::Super :
            if (hasCallStrike_) {
                if(longCallOption) {
                    callLeftEps_ = eps_;
                    callRightEps_ = 0.;
                } else {
                    callLeftEps_ = 0.;
                    callRightEps_ = eps_;
               }
            }
            if (hasPutStrike_) {
                if(longPutOption) {
                    putLeftEps_ = 0.;
                    putRightEps_ = eps_;
                } else {
                    putRightEps_ = 0.;
                    callRightEps_ = eps_;
                }
            }
            break;
          default:
            QL_FAIL("unsupported replication type");
        }
        
        registerWith(underlying);
    }

    Rate DigitalCoupon::callOptionRate() const {

        Rate callOptionRate = Rate(0.);
        if(hasCallStrike_) {      
            callOptionRate = isCashOrNothing_ ? digitalPayoff_ : underlying_->rate();
            CappedFlooredCoupon next(underlying_, callStrike_ + callRightEps_);
            CappedFlooredCoupon previous(underlying_, callStrike_ - callLeftEps_);
            callOptionRate *= (next.rate() - previous.rate()) / (callLeftEps_ + callRightEps_);
        }
        return callOptionRate;
    }

    Rate DigitalCoupon::putOptionRate() const {

        Rate putOptionRate = Rate(0.);
        if(hasPutStrike_) {      
            putOptionRate = isCashOrNothing_ ? digitalPayoff_ : underlying_->rate();
            CappedFlooredCoupon next(underlying_, Null<Rate>(), putStrike_ + putRightEps_);
            CappedFlooredCoupon previous(underlying_, Null<Rate>(), putStrike_ - putLeftEps_);
            putOptionRate *= (next.rate() - previous.rate()) / (putLeftEps_ + putRightEps_);
        }
        return putOptionRate;
    }

    Rate DigitalCoupon::rate() const {
        QL_REQUIRE(underlying_->pricer(), "pricer not set");
        return underlying_->rate() + callCsi_ * callOptionRate() + putCsi_ * putOptionRate();
    }

    Rate DigitalCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

    Rate DigitalCoupon::callStrike() const {
        if (hasCall())
            return callStrike_;
        else
            return Null<Rate>();
   }

    Rate DigitalCoupon::putStrike() const {
        if (hasPut())
            return putStrike_;
        else
            return Null<Rate>();
    }
    
    Rate DigitalCoupon::digitalPayoff() const {
        if (isCashOrNothing_)
            return digitalPayoff_;
        else
            return Null<Rate>();
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

