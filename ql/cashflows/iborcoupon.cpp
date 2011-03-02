/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2010 Ferdinando Ametrano

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    IborCoupon::IborCoupon(const Date& paymentDate,
                           Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           Natural fixingDays,
                           const boost::shared_ptr<IborIndex>& iborIndex,
                           Real gearing,
                           Spread spread,
                           const Date& refPeriodStart,
                           const Date& refPeriodEnd,
                           const DayCounter& dayCounter,
                           bool isInArrears)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, iborIndex, gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears),
      iborIndex_(iborIndex) {}

    Rate IborCoupon::indexFixing() const {

        #ifdef QL_USE_INDEXED_COUPON
        return index_->fixing(fixingDate());
        #else
        if (isInArrears()) {
            return index_->fixing(fixingDate());
        } else {
            Date today = Settings::instance().evaluationDate();
            Date fixing_date = fixingDate();
            if (fixing_date < today) {
                // must have been fixed
                Rate pastFixing = IndexManager::instance().getHistory(
                                                 index_->name())[fixing_date];
                QL_REQUIRE(pastFixing != Null<Real>(),
                           "Missing " << index_->name()
                           << " fixing for " << fixing_date);
                return pastFixing;
            }
            if (fixing_date == today) {
                // might have been fixed
                try {
                    Rate pastFixing = IndexManager::instance().getHistory(
                                                 index_->name())[fixing_date];
                    if (pastFixing != Null<Real>())
                        return pastFixing;
                    else
                        ;   // fall through and forecast
                } catch (Error&) {
                    ;       // fall through and forecast
                }
            }

            // forecast: 0) forecasting curve
            Handle<YieldTermStructure> termStructure =
                iborIndex_->forwardingTermStructure();
            QL_REQUIRE(!termStructure.empty(),
                       "null term structure set to this instance of " <<
                       index_->name());

            // forecast: 1) startDiscount
            Date fixingValueDate = index_->fixingCalendar().advance(
                fixing_date, index_->fixingDays(), Days);
            DiscountFactor startDiscount =
                termStructure->discount(fixingValueDate);
            // forecast: 2) endDiscount
            Date nextFixingDate = index_->fixingCalendar().advance(
                accrualEndDate_, -static_cast<Integer>(fixingDays()), Days);
            Date nextFixingValueDate = index_->fixingCalendar().advance(
                nextFixingDate, index_->fixingDays(), Days);
            DiscountFactor endDiscount =
                termStructure->discount(nextFixingValueDate);
            // forecast: 3) spanningTime
            Time spanningTime = index_->dayCounter().yearFraction(
                fixingValueDate, nextFixingValueDate);
            QL_REQUIRE(spanningTime>0.0,
                       "cannot calculate forward rate between " <<
                       fixingValueDate << " and " << nextFixingValueDate <<
                       ": non positive time using " << index_->dayCounter().name());
            // forecast: 4) implied fixing
            return (startDiscount/endDiscount-1.0)/spanningTime;
        }
    #endif

    }

    void IborCoupon::accept(AcyclicVisitor& v) {
        Visitor<IborCoupon>* v1 =
            dynamic_cast<Visitor<IborCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }



    IborLeg::IborLeg(const Schedule& schedule,
                     const boost::shared_ptr<IborIndex>& index)
    : schedule_(schedule), index_(index),
      paymentAdjustment_(Following),
      inArrears_(false), zeroPayments_(false) {}

    IborLeg& IborLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    IborLeg& IborLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    IborLeg& IborLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    IborLeg& IborLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    IborLeg& IborLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    IborLeg& IborLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    IborLeg& IborLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    IborLeg& IborLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    IborLeg& IborLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    IborLeg& IborLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    IborLeg& IborLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1,cap);
        return *this;
    }

    IborLeg& IborLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    IborLeg& IborLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1,floor);
        return *this;
    }

    IborLeg& IborLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

    IborLeg& IborLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    IborLeg& IborLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

    IborLeg::operator Leg() const {

        Leg leg = FloatingLeg<IborIndex, IborCoupon, CappedFlooredIborCoupon>(
                         schedule_, notionals_, index_, paymentDayCounter_,
                         paymentAdjustment_, fixingDays_, gearings_, spreads_,
                         caps_, floors_, inArrears_, zeroPayments_);

        if (caps_.empty() && floors_.empty() && !inArrears_)
            setCouponPricer(leg,
                            boost::shared_ptr<FloatingRateCouponPricer>(
                                                  new BlackIborCouponPricer));
        return leg;
    }

}
