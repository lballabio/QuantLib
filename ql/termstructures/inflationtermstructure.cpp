/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

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

#include <ql/termstructures/inflationtermstructure.hpp>

namespace QuantLib {

    InflationTermStructure::InflationTermStructure(
                                        const Period& lag,
                                        Frequency frequency,
                                        Rate baseRate,
                                        const Handle<YieldTermStructure>& yTS,
                                        const DayCounter& dayCounter)
    : TermStructure(dayCounter), nominalTermStructure_(yTS),
      lag_(lag), frequency_(frequency), baseRate_(baseRate) {
        registerWith(nominalTermStructure_);
    }

    InflationTermStructure::InflationTermStructure(
                                        const Date& referenceDate,
                                        const Period& lag,
                                        Frequency frequency,
                                        Rate baseRate,
                                        const Handle<YieldTermStructure>& yTS,
                                        const Calendar& calendar,
                                        const DayCounter& dayCounter)
    : TermStructure(referenceDate, calendar, dayCounter),
      nominalTermStructure_(yTS), lag_(lag),
      frequency_(frequency), baseRate_(baseRate) {
        registerWith(nominalTermStructure_);
    }

    InflationTermStructure::InflationTermStructure(
                                        Natural settlementDays,
                                        const Calendar& calendar,
                                        const Period& lag,
                                        Frequency frequency,
                                        Rate baseRate,
                                        const Handle<YieldTermStructure>& yTS,
                                        const DayCounter &dayCounter)
    : TermStructure(settlementDays, calendar, dayCounter),
      nominalTermStructure_(yTS), lag_(lag),
      frequency_(frequency), baseRate_(baseRate) {
        registerWith(nominalTermStructure_);
    }


    Period InflationTermStructure::lag() const {
        return lag_;
    }


    Frequency InflationTermStructure::frequency() const {
        return frequency_;
    }

    Handle<YieldTermStructure>
    InflationTermStructure::nominalTermStructure() const {
        return nominalTermStructure_;
    }

    Rate InflationTermStructure::baseRate() const {
        return baseRate_;
    }

    void InflationTermStructure::checkRange(const Date& d,
                                            bool extrapolate) const {
        QL_REQUIRE(d >= baseDate(),
                   "date (" << d << ") is before base date");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || d <= maxDate(),
                   "date (" << d << ") is past max curve date ("
                   << maxDate() << ")");
    }

    void InflationTermStructure::checkRange(Time t,
                                            bool extrapolate) const {
        QL_REQUIRE(t >= timeFromReference(baseDate()),
                   "time (" << t << ") is before base date");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
    }



    ZeroInflationTermStructure::ZeroInflationTermStructure(
                                    const DayCounter& dayCounter,
                                    const Period& lag,
                                    Frequency frequency,
                                    Rate baseZeroRate,
                                    const Handle<YieldTermStructure>& yTS)
    : InflationTermStructure(lag, frequency, baseZeroRate, yTS, dayCounter) {}

    ZeroInflationTermStructure::ZeroInflationTermStructure(
                                    const Date& referenceDate,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const Period& lag,
                                    Frequency frequency,
                                    Rate baseZeroRate,
                                    const Handle<YieldTermStructure>& yTS)
    : InflationTermStructure(referenceDate, lag, frequency, baseZeroRate,
                             yTS, calendar, dayCounter) {}

    ZeroInflationTermStructure::ZeroInflationTermStructure(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const Period& lag,
                                    Frequency frequency,
                                    Rate baseZeroRate,
                                    const Handle<YieldTermStructure>& yTS)
    : InflationTermStructure(settlementDays, calendar, lag, frequency,
                             baseZeroRate, yTS, dayCounter) {}

    Rate ZeroInflationTermStructure::zeroRate(const Date &d,
                                              bool extrapolate) const {
        InflationTermStructure::checkRange(d, extrapolate);
        return zeroRateImpl(timeFromReference(d));
    }

    Rate ZeroInflationTermStructure::zeroRate(Time t,
                                              bool extrapolate) const {
        InflationTermStructure::checkRange(t, extrapolate);
        return zeroRateImpl(t);
    }


    YoYInflationTermStructure::YoYInflationTermStructure(
                                    const DayCounter& dayCounter,
                                    const Period& lag,
                                    Frequency frequency,
                                    Rate baseYoYRate,
                                    const Handle<YieldTermStructure>& yTS)
    : InflationTermStructure(lag, frequency, baseYoYRate, yTS, dayCounter) {}

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    const Date& referenceDate,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const Period& lag,
                                    Frequency frequency,
                                    Rate baseYoYRate,
                                    const Handle<YieldTermStructure>& yTS)
    : InflationTermStructure(referenceDate, lag, frequency, baseYoYRate,
                             yTS, calendar, dayCounter) {}

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const Period& lag,
                                    Frequency frequency,
                                    Rate baseYoYRate,
                                    const Handle<YieldTermStructure>& yTS)
    : InflationTermStructure(settlementDays, calendar,lag, frequency,
                             baseYoYRate, yTS, dayCounter) {}

    Rate YoYInflationTermStructure::yoyRate(const Date &d,
                                            bool extrapolate) const {
        InflationTermStructure::checkRange(d, extrapolate);
        return yoyRateImpl(timeFromReference(d));
    }

    Rate YoYInflationTermStructure::yoyRate(Time t,
                                            bool extrapolate) const {
        InflationTermStructure::checkRange(t, extrapolate);
        return yoyRateImpl(t);
    }



    std::pair<Date,Date> inflationPeriod(const Date& d,
                                         Frequency frequency) {

        Month month = d.month();
        Year year = d.year();

        Month startMonth, endMonth;
        switch (frequency) {
          case Annual:
            startMonth = January;
            endMonth = December;
            break;
          case Semiannual:
            startMonth = Month((month-1)/6 + 1);
            endMonth = Month(startMonth + 5);
            break;
          case Quarterly:
            startMonth = Month((month-1)/3 + 1);
            endMonth = Month(startMonth + 2);
            break;
          case Monthly:
            startMonth = endMonth = month;
            break;
          default:
            QL_FAIL("Frequency not handled: " << frequency);
            break;
        };

        Date startDate = Date(1, startMonth, year);
        Date endDate = Date::endOfMonth(Date(1, endMonth, year));

        return std::make_pair(startDate,endDate);
    }

}

