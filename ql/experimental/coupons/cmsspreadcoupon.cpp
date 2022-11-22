/*
 Copyright (C) 2014 Peter Caspers

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
#include <ql/experimental/coupons/cmsspreadcoupon.hpp>
#include <utility>

namespace QuantLib {

    CmsSpreadCoupon::CmsSpreadCoupon(
        const Date &paymentDate, Real nominal, const Date &startDate,
        const Date &endDate, Natural fixingDays,
        const ext::shared_ptr<SwapSpreadIndex> &index, Real gearing,
        Spread spread, const Date &refPeriodStart,
        const Date &refPeriodEnd,
        const DayCounter &dayCounter, bool isInArrears, const Date &exCouponDate)
        : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                             fixingDays, index, gearing, spread,
                             refPeriodStart, refPeriodEnd, dayCounter,
                             isInArrears, exCouponDate),
          index_(index) {}

    void CmsSpreadCoupon::accept(AcyclicVisitor &v) {
        auto* v1 = dynamic_cast<Visitor<CmsSpreadCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    CmsSpreadLeg::CmsSpreadLeg(Schedule schedule, ext::shared_ptr<SwapSpreadIndex> index)
    : schedule_(std::move(schedule)), swapSpreadIndex_(std::move(index)) {
        QL_REQUIRE(swapSpreadIndex_, "no index provided");
    }

    CmsSpreadLeg &CmsSpreadLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1, notional);
        return *this;
    }

    CmsSpreadLeg &
    CmsSpreadLeg::withNotionals(const std::vector<Real> &notionals) {
        notionals_ = notionals;
        return *this;
    }

    CmsSpreadLeg &
    CmsSpreadLeg::withPaymentDayCounter(const DayCounter &dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    CmsSpreadLeg &
    CmsSpreadLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1, fixingDays);
        return *this;
    }

    CmsSpreadLeg &
    CmsSpreadLeg::withFixingDays(const std::vector<Natural> &fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1, gearing);
        return *this;
    }

    CmsSpreadLeg &
    CmsSpreadLeg::withGearings(const std::vector<Real> &gearings) {
        gearings_ = gearings;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    CmsSpreadLeg &
    CmsSpreadLeg::withSpreads(const std::vector<Spread> &spreads) {
        spreads_ = spreads;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1, cap);
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withCaps(const std::vector<Rate> &caps) {
        caps_ = caps;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1, floor);
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withFloors(const std::vector<Rate> &floors) {
        floors_ = floors;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    CmsSpreadLeg &CmsSpreadLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

    CmsSpreadLeg::operator Leg() const {
        return FloatingLeg<SwapSpreadIndex, CmsSpreadCoupon,
                           CappedFlooredCmsSpreadCoupon>(
            schedule_, notionals_, swapSpreadIndex_, paymentDayCounter_,
            paymentAdjustment_, fixingDays_, gearings_, spreads_, caps_,
            floors_, inArrears_, zeroPayments_);
    }
}
