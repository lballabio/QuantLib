/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/indexes/swapindex.hpp>
#include <utility>

namespace QuantLib {

    CmsCoupon::CmsCoupon(const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         Natural fixingDays,
                         const ext::shared_ptr<SwapIndex>& swapIndex,
                         Real gearing,
                         Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const DayCounter& dayCounter,
                         bool isInArrears,
                         const Date& exCouponDate)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, swapIndex, gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears, exCouponDate),
      swapIndex_(swapIndex) {}

    void CmsCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<CmsCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }


    CmsLeg::CmsLeg(Schedule schedule, ext::shared_ptr<SwapIndex> swapIndex)
    : schedule_(std::move(schedule)), swapIndex_(std::move(swapIndex)),
      paymentAdjustment_(Following), inArrears_(false), zeroPayments_(false) {}

    CmsLeg& CmsLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1, notional);
        return *this;
    }

    CmsLeg& CmsLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    CmsLeg& CmsLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    CmsLeg& CmsLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    CmsLeg& CmsLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1, fixingDays);
        return *this;
    }

    CmsLeg& CmsLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    CmsLeg& CmsLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1, gearing);
        return *this;
    }

    CmsLeg& CmsLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    CmsLeg& CmsLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    CmsLeg& CmsLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    CmsLeg& CmsLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1, cap);
        return *this;
    }

    CmsLeg& CmsLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    CmsLeg& CmsLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1, floor);
        return *this;
    }

    CmsLeg& CmsLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

    CmsLeg& CmsLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    CmsLeg& CmsLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

    CmsLeg& CmsLeg::withExCouponPeriod(
                                const Period& period,
                                const Calendar& cal,
                                BusinessDayConvention convention,
                                bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
    }

    CmsLeg::operator Leg() const {
        return FloatingLeg<SwapIndex, CmsCoupon, CappedFlooredCmsCoupon>(
                         schedule_, notionals_, swapIndex_, paymentDayCounter_,
                         paymentAdjustment_, fixingDays_, gearings_, spreads_,
                         caps_, floors_, inArrears_, zeroPayments_,
                         0, Calendar(),
                         exCouponPeriod_, exCouponCalendar_,
                         exCouponAdjustment_, exCouponEndOfMonth_);
   }

}
