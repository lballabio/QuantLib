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

#include <ql/cashflows/digitaliborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>

namespace QuantLib {

    DigitalIborCoupon::DigitalIborCoupon(
                      const boost::shared_ptr<IborCoupon>& underlying,
                      Rate callStrike,
                      Position::Type callPosition,
                      bool isCallATMIncluded,
                      Rate callDigitalPayoff,
                      Rate putStrike,
                      Position::Type putPosition,
                      bool isPutATMIncluded,
                      Rate putDigitalPayoff,
                      const boost::shared_ptr<DigitalReplication>& replication)
    : DigitalCoupon(underlying, callStrike, callPosition, isCallATMIncluded,
                    callDigitalPayoff, putStrike, putPosition,
                    isPutATMIncluded, putDigitalPayoff, replication) {}

    void DigitalIborCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        Visitor<DigitalIborCoupon>* v1 =
            dynamic_cast<Visitor<DigitalIborCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }



    DigitalIborLeg::DigitalIborLeg(const Schedule& schedule,
                                   const boost::shared_ptr<IborIndex>& index)
    : schedule_(schedule), index_(index),
      paymentAdjustment_(Following), inArrears_(false),
      longCallOption_(Position::Long), callATM_(false),
      longPutOption_(Position::Long), putATM_(false) {}

    DigitalIborLeg& DigitalIborLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withCallStrikes(Rate strike) {
        callStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withCallStrikes(
                                           const std::vector<Rate>& strikes) {
        callStrikes_ = strikes;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withLongCallOption(Position::Type type) {
        longCallOption_ = type;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withCallATM(bool flag) {
        callATM_ = flag;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withCallPayoffs(Rate payoff) {
        callPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withCallPayoffs(
                                           const std::vector<Rate>& payoffs) {
        callPayoffs_ = payoffs;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPutStrikes(Rate strike) {
        putStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPutStrikes(
                                           const std::vector<Rate>& strikes) {
        putStrikes_ = strikes;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withLongPutOption(Position::Type type) {
        longPutOption_ = type;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPutATM(bool flag) {
        putATM_ = flag;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPutPayoffs(Rate payoff) {
        putPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withPutPayoffs(
                                           const std::vector<Rate>& payoffs) {
        putPayoffs_ = payoffs;
        return *this;
    }

    DigitalIborLeg& DigitalIborLeg::withReplication(
                   const boost::shared_ptr<DigitalReplication>& replication) {
        replication_ = replication;
        return *this;
    }

    DigitalIborLeg::operator Leg() const {
        return FloatingDigitalLeg<IborIndex, IborCoupon, DigitalIborCoupon>(
                            schedule_, notionals_, index_, paymentDayCounter_,
                            paymentAdjustment_, fixingDays_,
                            gearings_, spreads_, inArrears_,
                            callStrikes_, longCallOption_,
                            callATM_, callPayoffs_,
                            putStrikes_, longPutOption_,
                            putATM_, putPayoffs_,
                            replication_);
    }

}
