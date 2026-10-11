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

    tradingTimeTermStructure::Jump::Jump(const Date& date, Real timeOfDay, Handle<Quote> size)
    : date_(date), timeOfDay_(timeOfDay), size_(std::move(size)) {
        QL_REQUIRE(date.fractionOfDay() == 0.0,
                   "jump date " << date << " carries a time of day; give the time once");
        QL_REQUIRE(timeOfDay > 0.0 && timeOfDay < 1.0,
                   "jump time of day must be in (0, 1), not at midnight: " << timeOfDay);
        QL_REQUIRE(!size_.empty(), "no jump size given");
    }

    tradingTimeTermStructure::Jump::Jump(const Date& dateTime, Handle<Quote> size)
    : date_(dateTime.serialNumber()), timeOfDay_(dateTime.fractionOfDay()), size_(std::move(size)) {
        QL_REQUIRE(timeOfDay_ > 0.0,
                   "jump on " << dateTime << " has no time of day: give it explicitly"
                              << " (a date carries one only with high-resolution dates)");
        QL_REQUIRE(!size_.empty(), "no jump size given");
    }

    tradingTimeTermStructure::tradingTimeTermStructure(const Date& referenceDate,
                                                       Calendar calendar,
                                                       Real weekendWeight,
                                                       std::vector<Jump> jumps)
    : TermStructure(referenceDate, std::move(calendar), Actual365Fixed()),
      weekendWeight_(weekendWeight), jumps_(std::move(jumps)) {
        QL_REQUIRE(weekendWeight_ >= 0.0 && weekendWeight_ <= 1.0,
                   "weekend weight must be in [0, 1]: " << weekendWeight_);
        for (const auto& j : jumps_)
            registerWith(j.size());
    }

    tradingTimeTermStructure::tradingTimeTermStructure(Natural settlementDays,
                                                       Calendar calendar,
                                                       Real weekendWeight,
                                                       std::vector<Jump> jumps)
    : TermStructure(settlementDays, std::move(calendar), Actual365Fixed()),
      weekendWeight_(weekendWeight), jumps_(std::move(jumps)) {
        QL_REQUIRE(weekendWeight_ >= 0.0 && weekendWeight_ <= 1.0,
                   "weekend weight must be in [0, 1]: " << weekendWeight_);
        for (const auto& j : jumps_)
            registerWith(j.size());
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
        jumpDays_.resize(jumps_.size());
        for (Size i = 0; i < jumps_.size(); ++i)
            jumpDays_[i] = static_cast<Real>(jumps_[i].date().serialNumber() - ref.serialNumber()) +
                           jumps_[i].timeOfDay() - referenceTimeOfDay_;
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
        for (Size i = 0; i < jumps_.size(); ++i) {
            const Real xj = jumpDays_[i];
            if (xj > 0.0 && xj > x1 && xj <= x2) {
                const Handle<Quote>& size = jumps_[i].size();
                QL_REQUIRE(size->isValid(), "invalid size of the " << io::ordinal(i + 1) << " jump");
                QL_REQUIRE(size->value() >= 0.0,
                           io::ordinal(i + 1) << " jump has a negative size: " << size->value());
                sum += size->value();
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
        for (Real xj : jumpDays_)
            if (xj > 0.0)
                times.push_back(xj / 365.0);
        return times;
    }

}
