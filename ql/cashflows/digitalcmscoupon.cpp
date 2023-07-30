/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/cashflows/digitalcmscoupon.hpp>
#include <utility>

namespace QuantLib {

    DigitalCmsCoupon::DigitalCmsCoupon(
                      const ext::shared_ptr<CmsCoupon>& underlying,
                      Rate callStrike,
                      Position::Type callPosition,
                      bool isCallATMIncluded,
                      Rate callDigitalPayoff,
                      Rate putStrike,
                      Position::Type putPosition,
                      bool isPutATMIncluded,
                      Rate putDigitalPayoff,
                      const ext::shared_ptr<DigitalReplication>& replication,
                      bool nakedOption)
    : DigitalCoupon(underlying, callStrike, callPosition, isCallATMIncluded,
                    callDigitalPayoff, putStrike, putPosition,
                    isPutATMIncluded, putDigitalPayoff, replication, nakedOption) {}

    void DigitalCmsCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        auto* v1 = dynamic_cast<Visitor<DigitalCmsCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            super::accept(v);
    }


    DigitalCmsLeg::DigitalCmsLeg(Schedule schedule,
                                 ext::shared_ptr<SwapIndex> index,
                                 ext::shared_ptr<FloatingRateCouponPricer> pricer)
    : schedule_(std::move(schedule)), index_(std::move(index)), pricer_(std::move(pricer)) {}

    DigitalCmsLeg& DigitalCmsLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withCallStrikes(Rate strike) {
        callStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withCallStrikes(
                                           const std::vector<Rate>& strikes) {
        callStrikes_ = strikes;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withLongCallOption(Position::Type type) {
        longCallOption_ = type;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withCallATM(bool flag) {
        callATM_ = flag;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withCallPayoffs(Rate payoff) {
        callPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withCallPayoffs(
                                           const std::vector<Rate>& payoffs) {
        callPayoffs_ = payoffs;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPutStrikes(Rate strike) {
        putStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPutStrikes(
                                           const std::vector<Rate>& strikes) {
        putStrikes_ = strikes;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withLongPutOption(Position::Type type) {
        longPutOption_ = type;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPutATM(bool flag) {
        putATM_ = flag;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPutPayoffs(Rate payoff) {
        putPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withPutPayoffs(
                                           const std::vector<Rate>& payoffs) {
        putPayoffs_ = payoffs;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withReplication(
                   const ext::shared_ptr<DigitalReplication>& replication) {
        replication_ = replication;
        return *this;
    }

    DigitalCmsLeg& DigitalCmsLeg::withNakedOption(bool nakedOption) {
        nakedOption_ = nakedOption;
        return *this;
    }

    DigitalCmsLeg::operator Leg() const {
        return FloatingDigitalLeg<SwapIndex, CmsCoupon, DigitalCmsCoupon>(
                            schedule_, notionals_, index_, paymentDayCounter_,
                            paymentAdjustment_, fixingDays_,
                            gearings_, spreads_, inArrears_,
                            callStrikes_, longCallOption_,
                            callATM_, callPayoffs_,
                            putStrikes_, longPutOption_,
                            putATM_, putPayoffs_,
                            replication_, nakedOption_, pricer_);
    }

}
