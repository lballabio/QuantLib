/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Toyin Akin
 Copyright (C) 2021 Marcin Rybacki

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/multipleresetscoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <cmath>

namespace QuantLib {

    MultipleResetsCoupon::MultipleResetsCoupon(const Date& paymentDate,
                                               Real nominal,
                                               const Schedule& resetSchedule,
                                               Natural fixingDays,
                                               const ext::shared_ptr<IborIndex>& index,
                                               Real gearing,
                                               Rate couponSpread,
                                               Rate rateSpread,
                                               const Date& refPeriodStart,
                                               const Date& refPeriodEnd,
                                               const DayCounter& dayCounter,
                                               const Date& exCouponDate)
    : FloatingRateCoupon(paymentDate, nominal,
                         resetSchedule.front(), resetSchedule.back(),
                         fixingDays, index, gearing, couponSpread,
                         refPeriodStart, refPeriodEnd, dayCounter,
                         false, exCouponDate),
      rateSpread_(rateSpread) {
        valueDates_ = resetSchedule.dates();

        // fixing dates
        n_ = valueDates_.size() - 1;
        if (fixingDays_ == 0) {
            fixingDates_ = std::vector<Date>(valueDates_.begin(), valueDates_.end() - 1);
        } else {
            fixingDates_.resize(n_);
            for (Size i = 0; i < n_; ++i)
                fixingDates_[i] = fixingDate(valueDates_[i]);
        }

        // accrual times of sub-periods
        dt_.resize(n_);
        const DayCounter& dc = index->dayCounter();
        for (Size i = 0; i < n_; ++i)
            dt_[i] = dc.yearFraction(valueDates_[i], valueDates_[i + 1]);
    }

    MultipleResetsCoupon::MultipleResetsCoupon(const Date& paymentDate,
                                               Real nominal,
                                               const Date& startDate,
                                               const Date& endDate,
                                               Natural fixingDays,
                                               const ext::shared_ptr<IborIndex>& index,
                                               Real gearing,
                                               Rate couponSpread,
                                               Rate rateSpread,
                                               const Date& refPeriodStart,
                                               const Date& refPeriodEnd,
                                               const DayCounter& dayCounter,
                                               const Date& exCouponDate)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, couponSpread,
                         refPeriodStart, refPeriodEnd, dayCounter,
                         false, exCouponDate),
      rateSpread_(rateSpread) {
        Schedule sch = MakeSchedule()
                           .from(startDate)
                           .to(endDate)
                           .withTenor(index->tenor())
                           .withCalendar(index->fixingCalendar())
                           .withConvention(index->businessDayConvention())
                           .backwards()
                           .endOfMonth(index->endOfMonth());
        valueDates_ = sch.dates();

        // fixing dates
        n_ = valueDates_.size() - 1;
        if (fixingDays_ == 0) {
            fixingDates_ = std::vector<Date>(valueDates_.begin(), valueDates_.end() - 1);
        } else {
            fixingDates_.resize(n_);
            for (Size i = 0; i < n_; ++i)
                fixingDates_[i] = fixingDate(valueDates_[i]);
        }

        // accrual of sub-periods
        dt_.resize(n_);
        const DayCounter& dc = index->dayCounter();
        for (Size i = 0; i < n_; ++i)
            dt_[i] = dc.yearFraction(valueDates_[i], valueDates_[i + 1]);
     }

    void MultipleResetsCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<MultipleResetsCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    Date MultipleResetsCoupon::fixingDate(const Date& valueDate) const {
        Date fixingDate =
            index_->fixingCalendar().advance(valueDate, -static_cast<Integer>(fixingDays_), Days);
        return fixingDate;
    }

    void MultipleResetsPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const MultipleResetsCoupon*>(&coupon);
        QL_REQUIRE(coupon_, "sub-periods coupon required");

        ext::shared_ptr<IborIndex> index = ext::dynamic_pointer_cast<IborIndex>(coupon_->index());
        if (!index) {
            // coupon was right, index is not
            QL_FAIL("IborIndex required");
        }

        QL_REQUIRE(coupon_->accrualPeriod() != 0.0, "null accrual period");

        const std::vector<Date>& fixingDates = coupon_->fixingDates();
        Size n = fixingDates.size();
        subPeriodFixings_.resize(n);

        for (Size i = 0; i < n; i++) {
            subPeriodFixings_[i] = index->fixing(fixingDates[i]) + coupon_->rateSpread();
        }
    }

    Real MultipleResetsPricer::swapletPrice() const {
        QL_FAIL("MultipleResetsPricer::swapletPrice not implemented");
    }

    Real MultipleResetsPricer::capletPrice(Rate) const {
        QL_FAIL("MultipleResetsPricer::capletPrice not implemented");
    }

    Rate MultipleResetsPricer::capletRate(Rate) const {
        QL_FAIL("MultipleResetsPricer::capletRate not implemented");
    }

    Real MultipleResetsPricer::floorletPrice(Rate) const {
        QL_FAIL("MultipleResetsPricer::floorletPrice not implemented");
    }

    Rate MultipleResetsPricer::floorletRate(Rate) const {
        QL_FAIL("MultipleResetsPricer::floorletRate not implemented");
    }

    Real AveragingMultipleResetsPricer::swapletRate() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Size nCount = subPeriodFixings_.size();
        const std::vector<Time>& subPeriodFractions = coupon_->dt();
        Real aggregateFactor = 0.0;
        for (Size i = 0; i < nCount; i++) {
            aggregateFactor += subPeriodFixings_[i] * subPeriodFractions[i];
        }

        Real rate = aggregateFactor / coupon_->accrualPeriod();
        return coupon_->gearing() * rate + coupon_->spread();
    }

    Real CompoundingMultipleResetsPricer::swapletRate() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Real compoundFactor = 1.0;
        const std::vector<Time>& subPeriodFractions = coupon_->dt();
        Size nCount = subPeriodFixings_.size();
        for (Size i = 0; i < nCount; i++) {
            compoundFactor *= (1.0 + subPeriodFixings_[i] * subPeriodFractions[i]);
        }

        Real rate = (compoundFactor - 1.0) / coupon_->accrualPeriod();
        return coupon_->gearing() * rate + coupon_->spread();
    }



    MultipleResetsLeg::MultipleResetsLeg(Schedule schedule,
                                         ext::shared_ptr<IborIndex> index,
                                         Size resetsPerCoupon)
    : schedule_(std::move(schedule)), index_(std::move(index)), resetsPerCoupon_(resetsPerCoupon),
      paymentCalendar_(schedule_.calendar()) {
        QL_REQUIRE(index_, "no index provided");
        QL_REQUIRE(!schedule_.empty(), "empty schedule provided");
        QL_REQUIRE((schedule_.size() - 1) % resetsPerCoupon_ == 0,
                   "number of resets per coupon does not divide exactly number of periods in schedule");
    }

    MultipleResetsLeg& MultipleResetsLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1, notional);
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withPaymentDayCounter(const DayCounter& dc) {
        paymentDayCounter_ = dc;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withPaymentLag(Integer lag) {
        paymentLag_ = lag;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1, fixingDays);
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1, gearing);
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withCouponSpreads(Spread spread) {
        couponSpreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withCouponSpreads(const std::vector<Spread>& spreads) {
        couponSpreads_ = spreads;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withRateSpreads(Spread spread) {
        rateSpreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withRateSpreads(const std::vector<Spread>& spreads) {
        rateSpreads_ = spreads;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withAveragingMethod(RateAveraging::Type averagingMethod) {
        averagingMethod_ = averagingMethod;
        return *this;
    }

    MultipleResetsLeg& MultipleResetsLeg::withExCouponPeriod(const Period& period,
                                                     const Calendar& cal,
                                                     BusinessDayConvention convention,
                                                     bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
    }

    MultipleResetsLeg::operator Leg() const {
        Leg cashflows;
        Calendar calendar = schedule_.calendar();

        Size n = (schedule_.size() - 1) / resetsPerCoupon_;
        QL_REQUIRE(!notionals_.empty(), "no notional given");
        QL_REQUIRE(notionals_.size() <= n,
                   "too many nominals (" << notionals_.size() << "), only " << n << " required");
        QL_REQUIRE(gearings_.size() <= n,
                   "too many gearings (" << gearings_.size() << "), only " << n << " required");
        QL_REQUIRE(couponSpreads_.size() <= n,
                   "too many coupon spreads (" << couponSpreads_.size() << "), only " << n << " required");
        QL_REQUIRE(rateSpreads_.size() <= n,
                   "too many rate spreads (" << rateSpreads_.size() << "), only " << n << " required");
        QL_REQUIRE(fixingDays_.size() <= n,
                   "too many fixing days (" << fixingDays_.size() << "), only " << n << " required");

        for (Size i = 0; i < n; ++i) {
            Date start = schedule_.date(i * resetsPerCoupon_);
            Date end = schedule_.date((i + 1) * resetsPerCoupon_);
            auto subSchedule = schedule_.after(start).until(end);
            Date paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);
            Date exCouponDate;
            if (exCouponPeriod_ != Period()) {
                if (exCouponCalendar_.empty()) {
                    exCouponDate = calendar.advance(paymentDate, -exCouponPeriod_,
                                                    exCouponAdjustment_, exCouponEndOfMonth_);
                } else {
                    exCouponDate = exCouponCalendar_.advance(
                        paymentDate, -exCouponPeriod_, exCouponAdjustment_, exCouponEndOfMonth_);
                }
            }

            cashflows.push_back(ext::make_shared<MultipleResetsCoupon>(
                paymentDate, detail::get(notionals_, i, notionals_.back()), subSchedule,
                detail::get(fixingDays_, i, index_->fixingDays()), index_,
                detail::get(gearings_, i, 1.0), detail::get(couponSpreads_, i, 0.0),
                detail::get(rateSpreads_, i, 0.0), start, end, paymentDayCounter_,
                exCouponDate));
        }

        switch (averagingMethod_) {
          case RateAveraging::Simple:
            setCouponPricer(cashflows, ext::make_shared<AveragingMultipleResetsPricer>());
            break;
          case RateAveraging::Compound:
            setCouponPricer(cashflows, ext::make_shared<CompoundingMultipleResetsPricer>());
            break;
          default:
            QL_FAIL("unknown compounding convention (" << Integer(averagingMethod_) << ")");
        }
        return cashflows;
    }


    QL_DEPRECATED_DISABLE_WARNING

    SubPeriodsLeg::SubPeriodsLeg(Schedule schedule, ext::shared_ptr<IborIndex> i)
    : schedule_(std::move(schedule)), index_(std::move(i)), paymentCalendar_(schedule_.calendar()) {
        QL_REQUIRE(index_, "no index provided");
    }

    SubPeriodsLeg& SubPeriodsLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1, notional);
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withPaymentDayCounter(const DayCounter& dc) {
        paymentDayCounter_ = dc;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withPaymentLag(Integer lag) {
        paymentLag_ = lag;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1, fixingDays);
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1, gearing);
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withCouponSpreads(Spread spread) {
        couponSpreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withCouponSpreads(const std::vector<Spread>& spreads) {
        couponSpreads_ = spreads;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withRateSpreads(Spread spread) {
        rateSpreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withRateSpreads(const std::vector<Spread>& spreads) {
        rateSpreads_ = spreads;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withAveragingMethod(RateAveraging::Type averagingMethod) {
        averagingMethod_ = averagingMethod;
        return *this;
    }

    SubPeriodsLeg& SubPeriodsLeg::withExCouponPeriod(const Period& period,
                                                     const Calendar& cal,
                                                     BusinessDayConvention convention,
                                                     bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
    }

    SubPeriodsLeg::operator Leg() const {
        Leg cashflows;
        Calendar calendar = schedule_.calendar();
        Date refStart, start, refEnd, end, exCouponDate;
        Date paymentDate;

        Size n = schedule_.size() - 1;
        QL_REQUIRE(!notionals_.empty(), "no notional given");
        QL_REQUIRE(notionals_.size() <= n,
                   "too many nominals (" << notionals_.size() << "), only " << n << " required");
        QL_REQUIRE(gearings_.size() <= n,
                   "too many gearings (" << gearings_.size() << "), only " << n << " required");
        QL_REQUIRE(couponSpreads_.size() <= n,
                   "too many coupon spreads (" << couponSpreads_.size() << "), only " << n << " required");
        QL_REQUIRE(rateSpreads_.size() <= n,
                   "too many rate spreads (" << rateSpreads_.size() << "), only " << n << " required");
        QL_REQUIRE(fixingDays_.size() <= n,
                   "too many fixing days (" << fixingDays_.size() << "), only " << n << " required");

        for (Size i = 0; i < n; ++i) {
            refStart = start = schedule_.date(i);
            refEnd = end = schedule_.date(i + 1);
            paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);

            if (i == 0 && schedule_.hasIsRegular() && !schedule_.isRegular(i + 1))
                refStart = calendar.adjust(end - schedule_.tenor(), paymentAdjustment_);
            if (i == n - 1 && schedule_.hasIsRegular() && !schedule_.isRegular(i + 1))
                refEnd = calendar.adjust(start + schedule_.tenor(), paymentAdjustment_);
            if (exCouponPeriod_ != Period()) {
                if (exCouponCalendar_.empty()) {
                    exCouponDate = calendar.advance(paymentDate, -exCouponPeriod_,
                                                    exCouponAdjustment_, exCouponEndOfMonth_);
                } else {
                    exCouponDate = exCouponCalendar_.advance(
                        paymentDate, -exCouponPeriod_, exCouponAdjustment_, exCouponEndOfMonth_);
                }
            }
            cashflows.push_back(ext::shared_ptr<CashFlow>(new MultipleResetsCoupon(
                paymentDate, detail::get(notionals_, i, notionals_.back()), start, end,
                detail::get(fixingDays_, i, index_->fixingDays()), index_,
                detail::get(gearings_, i, 1.0), detail::get(couponSpreads_, i, 0.0),
                detail::get(rateSpreads_, i, 0.0), refStart, refEnd, paymentDayCounter_,
                exCouponDate)));
        }

        switch (averagingMethod_) {
          case RateAveraging::Simple:
            setCouponPricer(cashflows, ext::make_shared<AveragingMultipleResetsPricer>());
            break;
          case RateAveraging::Compound:
            setCouponPricer(cashflows, ext::make_shared<CompoundingMultipleResetsPricer>());
            break;
          default:
            QL_FAIL("unknown compounding convention (" << Integer(averagingMethod_) << ")");
        }
        return cashflows;
    }

    QL_DEPRECATED_ENABLE_WARNING

}
