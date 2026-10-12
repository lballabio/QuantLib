/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace QuantLib {

    namespace {

        // the fraction of the day carried by a date; dates carry none
        // without high-resolution dates
        Real fractionOfDay(const Date& d) {
#ifdef QL_HIGH_RESOLUTION_DATE
            return d.fractionOfDay();
#else
            (void)d;
            return 0.0;
#endif
        }

        Real dayNumber(const Date& date, Real timeOfDay) {
            return static_cast<Real>(date.serialNumber()) + timeOfDay;
        }

        void requireNoTimeOfDay(const Date& date) {
            QL_REQUIRE(fractionOfDay(date) == 0.0,
                       "date " << date << " carries a time of day; give the time once");
        }

        // rates below this are rounding errors on a rate of zero
        const Real rateTolerance = 1.0e-10;

    }

    TradingTimeEvent::TradingTimeEvent(Real start, Real end, Handle<Quote> weight)
    : start_(start), end_(end), weight_(std::move(weight)) {
        QL_REQUIRE(start_ <= end_, "event ends before it starts");
        QL_REQUIRE(!weight_.empty(), "no event weight given");
    }

    JumpTradingTimeEvent::JumpTradingTimeEvent(const Date& date,
                                               Real timeOfDay,
                                               Handle<Quote> weight)
    : TradingTimeEvent(dayNumber(date, timeOfDay), dayNumber(date, timeOfDay), std::move(weight)) {
        requireNoTimeOfDay(date);
        QL_REQUIRE(timeOfDay > 0.0 && timeOfDay < 1.0,
                   "jump time of day must be in (0, 1), not at midnight: " << timeOfDay);
    }

    JumpTradingTimeEvent::JumpTradingTimeEvent(const Date& dateTime, Handle<Quote> weight)
    : TradingTimeEvent(dayNumber(dateTime, fractionOfDay(dateTime)),
                       dayNumber(dateTime, fractionOfDay(dateTime)),
                       std::move(weight)) {
        QL_REQUIRE(fractionOfDay(dateTime) > 0.0,
                   "jump on " << dateTime << " has no time of day: give it explicitly"
                              << " (a date carries one only with high-resolution dates)");
    }

    DiffuseTradingTimeEvent::DiffuseTradingTimeEvent(const Date& startDate,
                                                     Real startTimeOfDay,
                                                     const Date& endDate,
                                                     Real endTimeOfDay,
                                                     Handle<Quote> weight)
    : TradingTimeEvent(dayNumber(startDate, startTimeOfDay),
                       std::max(dayNumber(endDate, endTimeOfDay), dayNumber(startDate, startTimeOfDay)),
                       std::move(weight)) {
        requireNoTimeOfDay(startDate);
        requireNoTimeOfDay(endDate);
        QL_REQUIRE(startTimeOfDay >= 0.0 && startTimeOfDay < 1.0,
                   "start time of day must be in [0, 1): " << startTimeOfDay);
        QL_REQUIRE(endTimeOfDay >= 0.0 && endTimeOfDay < 1.0,
                   "end time of day must be in [0, 1): " << endTimeOfDay);
        QL_REQUIRE(dayNumber(startDate, startTimeOfDay) < dayNumber(endDate, endTimeOfDay),
                   "diffuse event must end after it starts: " << startDate << " + "
                       << startTimeOfDay << " to " << endDate << " + " << endTimeOfDay);
    }

    DiffuseTradingTimeEvent::DiffuseTradingTimeEvent(const Date& startDateTime,
                                                     const Date& endDateTime,
                                                     Handle<Quote> weight)
    : TradingTimeEvent(dayNumber(startDateTime, fractionOfDay(startDateTime)),
                       std::max(dayNumber(endDateTime, fractionOfDay(endDateTime)),
                                dayNumber(startDateTime, fractionOfDay(startDateTime))),
                       std::move(weight)) {
        QL_REQUIRE(fractionOfDay(startDateTime) > 0.0 && fractionOfDay(endDateTime) > 0.0,
                   "diffuse event from " << startDateTime << " to " << endDateTime
                       << " has no time of day: give it explicitly"
                       << " (a date carries one only with high-resolution dates)");
        QL_REQUIRE(startDateTime < endDateTime, "diffuse event must end after it starts: "
                                                    << startDateTime << " to " << endDateTime);
    }

    Real DiffuseTradingTimeEvent::released(Real day) const {
        if (day <= start())
            return 0.0;
        if (day >= end())
            return 1.0;
        return (day - start()) / (end() - start());
    }

    Real DiffuseTradingTimeEvent::releaseRate(Real day) const {
        return day > start() && day < end() ? 1.0 / (end() - start()) : 0.0;
    }


    tradingTimeTermStructure::tradingTimeTermStructure(
        const Date& referenceDate,
        Calendar calendar,
        Real holidayWeight,
        std::vector<ext::shared_ptr<TradingTimeEvent>> events)
    : TermStructure(referenceDate, std::move(calendar), Actual365Fixed()),
      holidayWeight_(holidayWeight), events_(std::move(events)) {
        QL_REQUIRE(holidayWeight_ >= 0.0 && holidayWeight_ <= 1.0,
                   "holiday weight must be in [0, 1]: " << holidayWeight_);
        for (const auto& e : events_) {
            QL_REQUIRE(e, "null trading-time event given");
            registerWith(e->weight());
        }
    }

    tradingTimeTermStructure::tradingTimeTermStructure(
        Calendar calendar,
        Real holidayWeight,
        std::vector<ext::shared_ptr<TradingTimeEvent>> events)
    : TermStructure(Null<Natural>(), std::move(calendar), Actual365Fixed()),
      holidayWeight_(holidayWeight), events_(std::move(events)) {
        QL_REQUIRE(holidayWeight_ >= 0.0 && holidayWeight_ <= 1.0,
                   "holiday weight must be in [0, 1]: " << holidayWeight_);
        for (const auto& e : events_) {
            QL_REQUIRE(e, "null trading-time event given");
            registerWith(e->weight());
        }
    }

    const Date& tradingTimeTermStructure::referenceDate() const {
        if (!moving_)
            return TermStructure::referenceDate();
        // the evaluation date converts to a Date by value, so keep a copy
        // to return a reference to; not moved to a business day, since
        // trading time runs from now, weekends included
        floatingReference_ = Settings::instance().evaluationDate();
        return floatingReference_;
    }

    void tradingTimeTermStructure::update() {
        // an event weight or the evaluation date may have changed; the
        // computed state is rebuilt lazily, see checkReference()
        upToDate_ = false;
        TermStructure::update();
    }

    void tradingTimeTermStructure::resetBaseDays() const {
        // no trading days before the start of the reference day
        cumulativeBaseDays_.assign(1, 0.0);
    }

    void tradingTimeTermStructure::checkReference() const {
        const Date& ref = referenceDate();
        if (upToDate_ && ref == latestReference_)
            return;

        // the base days depend only on the reference day, not on its
        // time of day or on the events
        if (cumulativeBaseDays_.empty() || ref.serialNumber() != referenceDay_)
            resetBaseDays();
        latestReference_ = ref;
        referenceDay_ = ref.serialNumber();
        referenceDayFraction_ = fractionOfDay(ref);
        referenceInstant_ = dayNumber(ref, referenceDayFraction_);

        // events fully released by the reference instant add nothing
        pendingEvents_.clear();
        weights_.clear();
        releasedAtReference_.clear();
        for (Size i = 0; i < events_.size(); ++i) {
            const TradingTimeEvent& e = *events_[i];
            if (e.end() <= referenceInstant_)
                continue;
            QL_REQUIRE(e.weight()->isValid(),
                       "invalid weight of the " << io::ordinal(i + 1) << " trading-time event");
            const Real w = e.weight()->value();
            QL_REQUIRE(w >= 0.0 || e.allowsNegativeWeight(),
                       io::ordinal(i + 1) << " trading-time event has a negative weight: " << w);
            pendingEvents_.push_back(i);
            weights_.push_back(w);
            releasedAtReference_.push_back(e.released(referenceInstant_));
        }
        checkRateIsNonNegative();
        upToDate_ = true;
    }

    void tradingTimeTermStructure::checkRateIsNonNegative() const {
        // only negative weights can make the rate negative, and only
        // while they are releasing
        std::vector<Real> breaks;
        Real first = QL_MAX_REAL, last = -QL_MAX_REAL;
        for (Size k = 0; k < pendingEvents_.size(); ++k) {
            const TradingTimeEvent& e = *events_[pendingEvents_[k]];
            breaks.push_back(e.start());
            breaks.push_back(e.end());
            if (weights_[k] < 0.0) {
                first = std::min(first, e.start());
                last = std::max(last, e.end());
            }
        }
        if (first > last)
            return;

        // the rate is constant between event starts and ends and day
        // boundaries, so checking it at the middle of each piece is exact
        breaks.push_back(referenceInstant_);
        for (Real day = std::floor(first) + 1.0; day < last; day += 1.0)
            breaks.push_back(day);
        std::sort(breaks.begin(), breaks.end());
        breaks.erase(std::unique(breaks.begin(), breaks.end()), breaks.end());
        for (Size j = 1; j < breaks.size(); ++j) {
            const Real a = std::max(breaks[j - 1], first), b = std::min(breaks[j], last);
            if (b <= a || b <= referenceInstant_)
                continue;
            const Real m = 0.5 * (a + b);
            const Date day(static_cast<Date::serial_type>(std::floor(m)));
            Real rate = baseRate(day);
            for (Size k = 0; k < pendingEvents_.size(); ++k)
                rate += weights_[k] * events_[pendingEvents_[k]]->releaseRate(m);
            QL_REQUIRE(rate >= -rateTolerance,
                       "trading time would run backwards on " << day << ": rate " << rate);
        }
    }

    Real tradingTimeTermStructure::daysFromReference(const Date& d) const {
        const Date& ref = referenceDate();
        QL_REQUIRE(d >= ref, "date (" << d << ") before the reference date (" << ref << ")");
        // the whole days separately, to keep the precision of the fractions
        return static_cast<Real>(d.serialNumber() - referenceDay_) +
               (fractionOfDay(d) - referenceDayFraction_);
    }

    Real tradingTimeTermStructure::baseRate(const Date& day) const {
        return calendar().isBusinessDay(day) ? 1.0 : holidayWeight_;
    }

    Real tradingTimeTermStructure::baseDaysSinceReferenceDay(Real y) const {
        // cumulativeBaseDays_[k] holds the base trading days before day k
        const Size k = static_cast<Size>(std::floor(y));
        const Date day0(referenceDay_);
        while (cumulativeBaseDays_.size() < k + 2) {
            const Size j = cumulativeBaseDays_.size() - 1;
            cumulativeBaseDays_.push_back(cumulativeBaseDays_.back() +
                                          baseRate(day0 + static_cast<Date::serial_type>(j)));
        }
        return cumulativeBaseDays_[k] +
               (y - static_cast<Real>(k)) * (cumulativeBaseDays_[k + 1] - cumulativeBaseDays_[k]);
    }

    Real tradingTimeTermStructure::tradingDaysSinceReference(Real x) const {
        Real days = baseDaysSinceReferenceDay(referenceDayFraction_ + x) -
                    baseDaysSinceReferenceDay(referenceDayFraction_);
        const Real instant = referenceInstant_ + x;
        for (Size k = 0; k < pendingEvents_.size(); ++k)
            days += weights_[k] *
                    (events_[pendingEvents_[k]]->released(instant) - releasedAtReference_[k]);
        return days;
    }

    Time tradingTimeTermStructure::tradingTime(Time t) const {
        QL_REQUIRE(t >= 0.0, "negative time: " << t);
        checkReference();
        return tradingDaysSinceReference(t * 365.0) / 365.0;
    }

    Time tradingTimeTermStructure::tradingTime(const Date& d1, const Date& d2) const {
        checkReference();
        if (d2 < d1)
            return -tradingTime(d2, d1);
        return (tradingDaysSinceReference(daysFromReference(d2)) -
                tradingDaysSinceReference(daysFromReference(d1))) / 365.0;
    }

    std::vector<std::pair<Time, Time>> tradingTimeTermStructure::eventIntervals() const {
        checkReference();
        std::vector<std::pair<Time, Time>> intervals;
        intervals.reserve(pendingEvents_.size());
        for (Size i : pendingEvents_) {
            const TradingTimeEvent& e = *events_[i];
            intervals.emplace_back(std::max(e.start() - referenceInstant_, 0.0) / 365.0,
                                   (e.end() - referenceInstant_) / 365.0);
        }
        return intervals;
    }

}
