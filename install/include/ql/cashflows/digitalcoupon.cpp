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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/indexes/interestrateindex.hpp>

namespace QuantLib {

    DigitalCoupon::DigitalCoupon(const ext::shared_ptr<FloatingRateCoupon>& underlying,
                                 Rate callStrike,
                                 Position::Type callPosition,
                                 bool isCallATMIncluded,
                                 Rate callDigitalPayoff,
                                 Rate putStrike,
                                 Position::Type putPosition,
                                 bool isPutATMIncluded,
                                 Rate putDigitalPayoff,
                                 ext::shared_ptr<DigitalReplication> replication,
                                 const bool nakedOption)
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
      underlying_(underlying), isCallATMIncluded_(isCallATMIncluded),
      isPutATMIncluded_(isPutATMIncluded), nakedOption_(nakedOption) {

        if (replication == nullptr)
            replication = ext::make_shared<DigitalReplication>();
        
        QL_REQUIRE(replication->gap()>0.0, "Non positive epsilon not allowed");

        callLeftEps_ = callRightEps_ = putLeftEps_ = putRightEps_ = replication->gap() / 2;
        replicationType_ = replication->replicationType();
        
        if (putStrike == Null<Rate>()) {
            QL_REQUIRE(putDigitalPayoff == Null<Rate>(),
            "Put Cash rate non allowed if put strike is null");
        }
        if (callStrike == Null<Rate>()) {
            QL_REQUIRE(callDigitalPayoff == Null<Rate>(),
            "Call Cash rate non allowed if call strike is null");
        }
        if (callStrike != Null<Rate>()) {
            hasCallStrike_ = true;
            callStrike_ = callStrike;
            switch (callPosition) {
                case Position::Long :
                    callCsi_ = 1.0;
                    break;
                case Position::Short :
                    callCsi_ = -1.0;
                    break;
                default:
                    QL_FAIL("unsupported position type");
            }
            if (callDigitalPayoff != Null<Rate>()){
                callDigitalPayoff_ = callDigitalPayoff;
                isCallCashOrNothing_ = true;
            }
        }
        if (putStrike != Null<Rate>()){
            hasPutStrike_ = true;
            putStrike_ = putStrike;
            switch (putPosition) {
                case Position::Long :
                    putCsi_ = 1.0;
                    break;
                case Position::Short :
                    putCsi_ = -1.0;
                    break;
                default:
                    QL_FAIL("unsupported position type");
            }
            if (putDigitalPayoff != Null<Rate>()){
                putDigitalPayoff_ = putDigitalPayoff;
                isPutCashOrNothing_ = true;
            }
        }

        switch (replicationType_) {
          case Replication::Central :
            // do nothing
            break;
          case Replication::Sub :
            if (hasCallStrike_) {
                switch (callPosition) {
                    case Position::Long :
                        callLeftEps_ = 0.;
                        callRightEps_ = replication->gap();
                        break;
                    case Position::Short :
                        callLeftEps_ = replication->gap();
                        callRightEps_ = 0.;
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            if (hasPutStrike_) {
                switch (putPosition) {
                    case Position::Long :
                        putLeftEps_ = replication->gap();
                        putRightEps_ = 0.;
                        break;
                    case Position::Short :
                        putLeftEps_ = 0.;
                        putRightEps_ = replication->gap();
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            break;
          case Replication::Super :
            if (hasCallStrike_) {
                switch (callPosition) {
                    case Position::Long :
                        callLeftEps_ = replication->gap();
                        callRightEps_ = 0.;
                        break;
                    case Position::Short :
                        callLeftEps_ = 0.;
                        callRightEps_ = replication->gap();
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            if (hasPutStrike_) {
                switch (putPosition) {
                    case Position::Long :
                        putLeftEps_ = 0.;
                        putRightEps_ = replication->gap();
                        break;
                    case Position::Short :
                        putLeftEps_ = replication->gap();
                        putRightEps_ = 0.;
                        break;
                    default:
                        QL_FAIL("unsupported position type");
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
            // Step function
            callOptionRate = isCallCashOrNothing_ ? callDigitalPayoff_ : callStrike_;
            CappedFlooredCoupon next(underlying_, callStrike_ + callRightEps_);
            CappedFlooredCoupon previous(underlying_, callStrike_ - callLeftEps_);
            callOptionRate *= (next.rate() - previous.rate())
                            / (callLeftEps_ + callRightEps_);
            if (!isCallCashOrNothing_) {
                // Call
                CappedFlooredCoupon atStrike(underlying_, callStrike_);
                Rate call = underlying_->rate() - atStrike.rate();
                // Sum up
                callOptionRate += call;
            }
        }
        return callOptionRate;
    }

    Rate DigitalCoupon::putOptionRate() const {

        Rate putOptionRate = Rate(0.);
        if(hasPutStrike_) {
            // Step function
            putOptionRate = isPutCashOrNothing_ ? putDigitalPayoff_ : putStrike_;
            CappedFlooredCoupon next(underlying_, Null<Rate>(), putStrike_ + putRightEps_);
            CappedFlooredCoupon previous(underlying_, Null<Rate>(), putStrike_ - putLeftEps_);
            putOptionRate *= (next.rate() - previous.rate())
                           / (putLeftEps_ + putRightEps_);
            if (!isPutCashOrNothing_) {
                // Put
                CappedFlooredCoupon atStrike(underlying_, Null<Rate>(), putStrike_);
                Rate put = - underlying_->rate() + atStrike.rate();
                // Sum up
                putOptionRate -= put;
            }
        }
        return putOptionRate;
    }

    void DigitalCoupon::deepUpdate() {
        update();
        underlying_->deepUpdate();
    }

    void DigitalCoupon::performCalculations() const {

        QL_REQUIRE(underlying_->pricer(), "pricer not set");

        Date fixingDate = underlying_->fixingDate();
        Date today = Settings::instance().evaluationDate();
        bool enforceTodaysHistoricFixings =
            Settings::instance().enforcesTodaysHistoricFixings();
        Rate underlyingRate = nakedOption_ ? 0.0 : underlying_->rate();
        if (fixingDate < today ||
            ((fixingDate == today) && enforceTodaysHistoricFixings)) {
            // must have been fixed
            rate_ = underlyingRate + callCsi_ * callPayoff() + putCsi_  * putPayoff();
        } else if (fixingDate == today) {
            // might have been fixed
            if (underlying_->index()->hasHistoricalFixing(fixingDate)) {
                rate_ = underlyingRate + callCsi_ * callPayoff() + putCsi_  * putPayoff();
            } else {
                rate_ = underlyingRate + callCsi_ * callOptionRate() + putCsi_ * putOptionRate();
            }
        } else {
            rate_ = underlyingRate + callCsi_ * callOptionRate() + putCsi_ * putOptionRate();
        }
    }

    Rate DigitalCoupon::rate() const {
        calculate();
        return rate_;
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

    Rate DigitalCoupon::callDigitalPayoff() const {
        if (isCallCashOrNothing_)
            return callDigitalPayoff_;
        else
            return Null<Rate>();
    }

    Rate DigitalCoupon::putDigitalPayoff() const {
        if (isPutCashOrNothing_)
            return putDigitalPayoff_;
        else
            return Null<Rate>();
    }

    void DigitalCoupon::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        auto* v1 = dynamic_cast<Visitor<DigitalCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            super::accept(v);
    }

    Rate DigitalCoupon::callPayoff() const {
        // to use only if index has fixed
        Rate payoff(0.);
        if(hasCallStrike_) {
            Rate underlyingRate = underlying_->rate();
            if ( (underlyingRate - callStrike_) > 1.e-16 ) {
                payoff = isCallCashOrNothing_ ? callDigitalPayoff_ : underlyingRate;
            } else {
                if (isCallATMIncluded_) {
                    if ( std::abs(callStrike_ - underlyingRate) <= 1.e-16 )
                        payoff = isCallCashOrNothing_ ? callDigitalPayoff_ : underlyingRate;
                }
            }
        }
        return payoff;
    }

    Rate DigitalCoupon::putPayoff() const {
        // to use only if index has fixed
        Rate payoff(0.);
        if(hasPutStrike_) {
            Rate underlyingRate = underlying_->rate();
            if ( (putStrike_ - underlyingRate) > 1.e-16 ) {
                payoff = isPutCashOrNothing_ ? putDigitalPayoff_ : underlyingRate;
            } else {
                // putStrike_ <= underlyingRate
                if (isPutATMIncluded_) {
                    if ( std::abs(putStrike_ - underlyingRate) <= 1.e-16 )
                        payoff = isPutCashOrNothing_ ? putDigitalPayoff_ : underlyingRate;
                }
            }
        }
        return payoff;
    }

}

