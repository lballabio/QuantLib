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
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    namespace {

        void checkInputs(Real weekendWeight,
                         const std::vector<Handle<Quote>>& events,
                         const std::vector<Date>& eventDates,
                         const std::vector<Real>& eventTimesOfDay) {
            QL_REQUIRE(weekendWeight >= 0.0 && weekendWeight <= 1.0,
                       "weekend weight must be in [0, 1]: " << weekendWeight);
            QL_REQUIRE(eventDates.size() == events.size(),
                       "mismatch between number of events (" << events.size()
                           << ") and event dates (" << eventDates.size() << ")");
            QL_REQUIRE(eventTimesOfDay.empty() || eventTimesOfDay.size() == events.size(),
                       "mismatch between number of events (" << events.size()
                           << ") and event times of day (" << eventTimesOfDay.size() << ")");
            for (Real tod : eventTimesOfDay)
                QL_REQUIRE(tod >= 0.0 && tod < 1.0,
                           "event time of day must be a fraction of the day in [0, 1): " << tod);
        }

    }

    tradingTimeTermStructure::tradingTimeTermStructure(const Date& referenceDate,
                                                       Calendar calendar,
                                                       Real weekendWeight,
                                                       std::vector<Handle<Quote>> events,
                                                       std::vector<Date> eventDates,
                                                       std::vector<Real> eventTimesOfDay)
    : TermStructure(referenceDate, std::move(calendar), Actual365Fixed()),
      weekendWeight_(weekendWeight), events_(std::move(events)),
      eventDates_(std::move(eventDates)), eventTimesOfDay_(std::move(eventTimesOfDay)) {
        checkInputs(weekendWeight_, events_, eventDates_, eventTimesOfDay_);
        for (const auto& e : events_)
            registerWith(e);
    }

    tradingTimeTermStructure::tradingTimeTermStructure(Natural settlementDays,
                                                       Calendar calendar,
                                                       Real weekendWeight,
                                                       std::vector<Handle<Quote>> events,
                                                       std::vector<Date> eventDates,
                                                       std::vector<Real> eventTimesOfDay)
    : TermStructure(settlementDays, std::move(calendar), Actual365Fixed()),
      weekendWeight_(weekendWeight), events_(std::move(events)),
      eventDates_(std::move(eventDates)), eventTimesOfDay_(std::move(eventTimesOfDay)) {
        checkInputs(weekendWeight_, events_, eventDates_, eventTimesOfDay_);
        for (const auto& e : events_)
            registerWith(e);
    }

    void tradingTimeTermStructure::update() {
        // the computed state follows the reference date lazily, see checkReference()
        TermStructure::update();
    }

    void tradingTimeTermStructure::checkReference() const {
        const Date& ref = referenceDate();
        if (ref == latestReference_ && latestReference_ != Date())
            return;
        latestReference_ = ref;
        referenceTimeOfDay_ = ref.fractionOfDay();
        cumulative_.assign(1, 0.0);
        eventDays_.resize(events_.size());
        for (Size i = 0; i < events_.size(); ++i) {
            const Real timeOfDay =
                eventTimesOfDay_.empty() ? eventDates_[i].fractionOfDay() : eventTimesOfDay_[i];
            eventDays_[i] = static_cast<Real>(eventDates_[i].serialNumber() - ref.serialNumber()) +
                            timeOfDay - referenceTimeOfDay_;
        }
    }

    Real tradingTimeTermStructure::daysFromReferenceDay(const Date& d) const {
        const Date& ref = referenceDate();
        QL_REQUIRE(d >= ref, "date (" << d << ") before the reference date (" << ref << ")");
        return static_cast<Real>(d.serialNumber() - ref.serialNumber()) + d.fractionOfDay();
    }

    Real tradingTimeTermStructure::tradingDays(Real y) const {
        // the rate is 1 on business days and the weekend weight on other
        // days; cumulative_[k] holds the trading days before day k
        const Size k = static_cast<Size>(std::floor(y));
        const Date refDay = Date(referenceDate().serialNumber());
        while (cumulative_.size() < k + 2) {
            const Size j = cumulative_.size() - 1;
            const Date day = refDay + static_cast<Date::serial_type>(j);
            cumulative_.push_back(cumulative_.back() +
                                  (calendar().isBusinessDay(day) ? 1.0 : weekendWeight_));
        }
        return cumulative_[k] + (y - static_cast<Real>(k)) * (cumulative_[k + 1] - cumulative_[k]);
    }

    Real tradingTimeTermStructure::jumpDays(Real x1, Real x2) const {
        Real sum = 0.0;
        for (Size i = 0; i < events_.size(); ++i) {
            const Real xe = eventDays_[i];
            if (xe > 0.0 && xe > x1 && xe <= x2) {
                QL_REQUIRE(events_[i]->isValid(), "invalid " << io::ordinal(i + 1) << " event quote");
                const Real size = events_[i]->value();
                QL_REQUIRE(size >= 0.0, io::ordinal(i + 1) << " event has a negative size: " << size);
                sum += size;
            }
        }
        return sum;
    }

    Time tradingTimeTermStructure::tradingTime(Time t) const {
        QL_REQUIRE(t >= 0.0, "negative time: " << t);
        checkReference();
        const Real x = t * 365.0;
        const Real days = tradingDays(x + referenceTimeOfDay_) - tradingDays(referenceTimeOfDay_) +
                          jumpDays(0.0, x);
        return days / 365.0;
    }

    Time tradingTimeTermStructure::tradingTime(const Date& d1, const Date& d2) const {
        checkReference();
        if (d2 < d1)
            return -tradingTime(d2, d1);
        const Real y1 = daysFromReferenceDay(d1), y2 = daysFromReferenceDay(d2);
        const Real days = tradingDays(y2) - tradingDays(y1) +
                          jumpDays(y1 - referenceTimeOfDay_, y2 - referenceTimeOfDay_);
        return days / 365.0;
    }

    std::vector<Time> tradingTimeTermStructure::jumpTimes() const {
        checkReference();
        std::vector<Time> times;
        for (Real xe : eventDays_)
            if (xe > 0.0)
                times.push_back(xe / 365.0);
        return times;
    }

}
