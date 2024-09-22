/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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


#include <ql/cashflows/capflooredinflationcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>
#include <utility>

namespace QuantLib {

    YoYInflationCoupon::
    YoYInflationCoupon(const Date& paymentDate,
                       Real nominal,
                       const Date& startDate,
                       const Date& endDate,
                       Natural fixingDays,
                       const ext::shared_ptr<YoYInflationIndex>& yoyIndex,
                       const Period& observationLag,
                       CPI::InterpolationType interpolation,
                       const DayCounter& dayCounter,
                       Real gearing,
                       Spread spread,
                       const Date& refPeriodStart,
                       const Date& refPeriodEnd)
    : InflationCoupon(paymentDate, nominal, startDate, endDate,
                      fixingDays, yoyIndex, observationLag,
                      dayCounter, refPeriodStart, refPeriodEnd),
      yoyIndex_(yoyIndex), interpolation_(interpolation), gearing_(gearing), spread_(spread) {}

    YoYInflationCoupon::
    YoYInflationCoupon(const Date& paymentDate,
                       Real nominal,
                       const Date& startDate,
                       const Date& endDate,
                       Natural fixingDays,
                       const ext::shared_ptr<YoYInflationIndex>& yoyIndex,
                       const Period& observationLag,
                       const DayCounter& dayCounter,
                       Real gearing,
                       Spread spread,
                       const Date& refPeriodStart,
                       const Date& refPeriodEnd)
    : YoYInflationCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, yoyIndex, observationLag, CPI::AsIndex,
                         dayCounter, gearing, spread, refPeriodStart, refPeriodEnd) {}


    void YoYInflationCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<YoYInflationCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            InflationCoupon::accept(v);
    }

    bool YoYInflationCoupon::checkPricerImpl(
            const ext::shared_ptr<InflationCouponPricer>&pricer) const {
        return static_cast<bool>(
               ext::dynamic_pointer_cast<YoYInflationCouponPricer>(pricer));
    }

    Rate YoYInflationCoupon::indexFixing() const {
        return CPI::laggedYoYRate(yoyIndex(), accrualEndDate(), observationLag(), interpolation_);
    }


    yoyInflationLeg::yoyInflationLeg(Schedule schedule,
                                     Calendar paymentCalendar,
                                     ext::shared_ptr<YoYInflationIndex> index,
                                     const Period& observationLag,
                                     CPI::InterpolationType interpolation)
    : schedule_(std::move(schedule)), index_(std::move(index)), observationLag_(observationLag),
      interpolation_(interpolation), paymentCalendar_(std::move(paymentCalendar)) {}

    yoyInflationLeg::yoyInflationLeg(Schedule schedule,
                                     Calendar paymentCalendar,
                                     ext::shared_ptr<YoYInflationIndex> index,
                                     const Period& observationLag)
    : yoyInflationLeg(schedule, paymentCalendar, index, observationLag, CPI::AsIndex) {}


    yoyInflationLeg& yoyInflationLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1,cap);
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1,floor);
        return *this;
    }

    yoyInflationLeg& yoyInflationLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }


    yoyInflationLeg::operator Leg() const {

        Size n = schedule_.size()-1;
        QL_REQUIRE(!paymentDayCounter_.empty(), "no payment daycounter given");
        QL_REQUIRE(!notionals_.empty(), "no notional given");
        QL_REQUIRE(notionals_.size() <= n,
                   "too many nominals (" << notionals_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(gearings_.size()<=n,
                   "too many gearings (" << gearings_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(spreads_.size()<=n,
                   "too many spreads (" << spreads_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(caps_.size()<=n,
                   "too many caps (" << caps_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(floors_.size()<=n,
                   "too many floors (" << floors_.size() <<
                   "), only " << n << " required");

        Leg leg; leg.reserve(n);

        Calendar calendar = paymentCalendar_;

        Date refStart, start, refEnd, end;

        for (Size i=0; i<n; ++i) {
            refStart = start = schedule_.date(i);
            refEnd   =   end = schedule_.date(i+1);
            Date paymentDate = calendar.adjust(end, paymentAdjustment_);
            if (i==0 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1)) {
                BusinessDayConvention bdc = schedule_.businessDayConvention();
                refStart = schedule_.calendar().adjust(end - schedule_.tenor(), bdc);
            }
            if (i==n-1 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1)) {
                BusinessDayConvention bdc = schedule_.businessDayConvention();
                refEnd = schedule_.calendar().adjust(start + schedule_.tenor(), bdc);
            }
            if (detail::get(gearings_, i, 1.0) == 0.0) { // fixed coupon
                leg.push_back(ext::make_shared<FixedRateCoupon>(
                            paymentDate,
                            detail::get(notionals_, i, 1.0),
                            detail::effectiveFixedRate(spreads_,caps_,
                                                       floors_,i),
                            paymentDayCounter_,
                            start, end, refStart, refEnd));
            } else { // yoy inflation coupon
                if (detail::noOption(caps_, floors_, i)) { // just swaplet
                    leg.push_back(ext::make_shared<YoYInflationCoupon>(
                            paymentDate,
                            detail::get(notionals_, i, 1.0),
                            start, end,
                            detail::get(fixingDays_, i, 0),
                            index_,
                            observationLag_,
                            interpolation_,
                            paymentDayCounter_,
                            detail::get(gearings_, i, 1.0),
                            detail::get(spreads_, i, 0.0),
                            refStart, refEnd));
                } else {    // cap/floorlet
                    leg.push_back(ext::make_shared<CappedFlooredYoYInflationCoupon>(
                            paymentDate,
                            detail::get(notionals_, i, 1.0),
                            start, end,
                            detail::get(fixingDays_, i, 0),
                            index_,
                            observationLag_,
                            interpolation_,
                            paymentDayCounter_,
                            detail::get(gearings_, i, 1.0),
                            detail::get(spreads_, i, 0.0),
                            detail::get(caps_,   i, Null<Rate>()),
                            detail::get(floors_, i, Null<Rate>()),
                            refStart, refEnd));
                }
            }
        }

        // Without caps or floors, this is enough; otherwise, a more
        // specific pricer will need to be set in client code.
        if (caps_.empty() && floors_.empty())
            setCouponPricer(leg, ext::make_shared<YoYInflationCouponPricer>());

        return leg;
    }

}

