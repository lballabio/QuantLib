/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/experimental/coupons/digitalcmsspreadcoupon.hpp>
#include <utility>

namespace QuantLib {

    DigitalCmsSpreadCoupon::DigitalCmsSpreadCoupon(
                      const std::shared_ptr<CmsSpreadCoupon>& underlying,
                      Rate callStrike,
                      Position::Type callPosition,
                      bool isCallATMIncluded,
                      Rate callDigitalPayoff,
                      Rate putStrike,
                      Position::Type putPosition,
                      bool isPutATMIncluded,
                      Rate putDigitalPayoff,
                      const std::shared_ptr<DigitalReplication>& replication,
                      bool nakedOption)
    : DigitalCoupon(underlying, callStrike, callPosition, isCallATMIncluded,
                    callDigitalPayoff, putStrike, putPosition,
                    isPutATMIncluded, putDigitalPayoff, replication, nakedOption) {}

    void DigitalCmsSpreadCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        auto* v1 = dynamic_cast<Visitor<DigitalCmsSpreadCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            super::accept(v);
    }


    DigitalCmsSpreadLeg::DigitalCmsSpreadLeg(Schedule schedule,
                                             std::shared_ptr<SwapSpreadIndex> index)
    : schedule_(std::move(schedule)), index_(std::move(index)) {}

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallStrikes(Rate strike) {
        callStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallStrikes(
                                           const std::vector<Rate>& strikes) {
        callStrikes_ = strikes;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withLongCallOption(Position::Type type) {
        longCallOption_ = type;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallATM(bool flag) {
        callATM_ = flag;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallPayoffs(Rate payoff) {
        callPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallPayoffs(
                                           const std::vector<Rate>& payoffs) {
        callPayoffs_ = payoffs;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutStrikes(Rate strike) {
        putStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutStrikes(
                                           const std::vector<Rate>& strikes) {
        putStrikes_ = strikes;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withLongPutOption(Position::Type type) {
        longPutOption_ = type;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutATM(bool flag) {
        putATM_ = flag;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutPayoffs(Rate payoff) {
        putPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutPayoffs(
                                           const std::vector<Rate>& payoffs) {
        putPayoffs_ = payoffs;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withReplication(
                   const std::shared_ptr<DigitalReplication>& replication) {
        replication_ = replication;
        return *this;
    }

    DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withNakedOption(bool nakedOption) {
        nakedOption_ = nakedOption;
        return *this;
    }

    DigitalCmsSpreadLeg::operator Leg() const {
        return FloatingDigitalLeg<SwapSpreadIndex, CmsSpreadCoupon, DigitalCmsSpreadCoupon>(
                            schedule_, notionals_, index_, paymentDayCounter_,
                            paymentAdjustment_, fixingDays_,
                            gearings_, spreads_, inArrears_,
                            callStrikes_, longCallOption_,
                            callATM_, callPayoffs_,
                            putStrikes_, longPutOption_,
                            putATM_, putPayoffs_,
                            replication_, nakedOption_);
    }

}
