/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon

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

#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <utility>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    InflationTermStructure::InflationTermStructure(
                                        Date baseDate,
                                        Frequency frequency,
                                        const DayCounter& dayCounter,
                                        ext::shared_ptr<Seasonality> seasonality,
                                        Rate baseRate)
    : TermStructure(dayCounter), seasonality_(std::move(seasonality)),
      frequency_(frequency), baseRate_(baseRate), baseDate_(baseDate) {
        if (seasonality_ != nullptr) {
            QL_REQUIRE(seasonality_->isConsistent(*this),
                       "Seasonality inconsistent with inflation term structure");
        }
    }

    InflationTermStructure::InflationTermStructure(
                                        const Date& referenceDate,
                                        Date baseDate,
                                        Frequency frequency,
                                        const DayCounter& dayCounter,
                                        ext::shared_ptr<Seasonality> seasonality,
                                        Rate baseRate)
    : TermStructure(referenceDate, Calendar(), dayCounter), seasonality_(std::move(seasonality)),
      frequency_(frequency), baseRate_(baseRate), baseDate_(baseDate) {
        if (seasonality_ != nullptr) {
            QL_REQUIRE(seasonality_->isConsistent(*this),
                       "Seasonality inconsistent with inflation term structure");
        }
    }

    InflationTermStructure::InflationTermStructure(
                                        Natural settlementDays,
                                        const Calendar& calendar,
                                        Date baseDate,
                                        Frequency frequency,
                                        const DayCounter& dayCounter,
                                        ext::shared_ptr<Seasonality> seasonality,
                                        Rate baseRate)
    : TermStructure(settlementDays, calendar, dayCounter), seasonality_(std::move(seasonality)),
      frequency_(frequency), baseRate_(baseRate), baseDate_(baseDate) {
        if (seasonality_ != nullptr) {
            QL_REQUIRE(seasonality_->isConsistent(*this),
                       "Seasonality inconsistent with inflation term structure");
        }
    }

    QL_DEPRECATED_ENABLE_WARNING

    Date InflationTermStructure::baseDate() const {
        return baseDate_;
    }

    void InflationTermStructure::setSeasonality(
                          const ext::shared_ptr<Seasonality>& seasonality) {
        // always reset, whether with null or new pointer
        seasonality_ = seasonality;
        if (seasonality_ != nullptr) {
            QL_REQUIRE(seasonality_->isConsistent(*this),
                       "Seasonality inconsistent with inflation term structure");
        }
        update();
    }


    void InflationTermStructure::checkRange(const Date& d,
                                            bool extrapolate) const {
        QL_REQUIRE(d >= baseDate(),
                   "date (" << d << ") is before base date (" << baseDate() << ")");
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
                                   Date baseDate,
                                   Frequency frequency,
                                   const DayCounter& dayCounter,
                                   const ext::shared_ptr<Seasonality>& seasonality)
    : InflationTermStructure(baseDate, frequency, dayCounter, seasonality) {}

    ZeroInflationTermStructure::ZeroInflationTermStructure(
                                   const Date& referenceDate,
                                   Date baseDate,
                                   Frequency frequency,
                                   const DayCounter& dayCounter,
                                   const ext::shared_ptr<Seasonality>& seasonality)
    : InflationTermStructure(referenceDate, baseDate, frequency, dayCounter, seasonality) {}

    ZeroInflationTermStructure::ZeroInflationTermStructure(
                                   Natural settlementDays,
                                   const Calendar& calendar,
                                   Date baseDate,
                                   Frequency frequency,
                                   const DayCounter& dayCounter,
                                   const ext::shared_ptr<Seasonality>& seasonality)
    : InflationTermStructure(settlementDays, calendar, baseDate, frequency, dayCounter, seasonality) {}

    Rate ZeroInflationTermStructure::zeroRate(const Date &d, const Period& instObsLag,
                                              bool forceLinearInterpolation,
                                              bool extrapolate) const {

        Period useLag = instObsLag;
        if (instObsLag == Period(-1,Days)) {
            useLag = Period(0, Days);
        }

        Rate zeroRate;
        if (forceLinearInterpolation) {
            std::pair<Date,Date> dd = inflationPeriod(d-useLag, frequency());
            dd.second = dd.second + Period(1,Days);
            Real dp = dd.second - dd.first;
            Real dt = d - dd.first;
            // if we are interpolating we only check the exact point
            // this prevents falling off the end at curve maturity
            InflationTermStructure::checkRange(d, extrapolate);
            Time t1 = timeFromReference(dd.first);
            Time t2 = timeFromReference(dd.second);
            Rate z1 = zeroRateImpl(t1);
            Rate z2 = zeroRateImpl(t2);
            zeroRate = z1 + (z2-z1) * (dt/dp);
        } else {
            std::pair<Date,Date> dd = inflationPeriod(d-useLag, frequency());
            InflationTermStructure::checkRange(dd.first, extrapolate);
            Time t = timeFromReference(dd.first);
            zeroRate = zeroRateImpl(t);
        }

        if (hasSeasonality()) {
            zeroRate = seasonality()->correctZeroRate(d-useLag, zeroRate, *this);
        }
        return zeroRate;
    }

    Rate ZeroInflationTermStructure::zeroRate(Time t,
                                              bool extrapolate) const {
        checkRange(t, extrapolate);
        return zeroRateImpl(t);
    }


    QL_DEPRECATED_DISABLE_WARNING

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    Date baseDate,
                                    Rate baseYoYRate,
                                    Frequency frequency,
                                    const DayCounter& dayCounter,
                                    const ext::shared_ptr<Seasonality> &seasonality)
    : InflationTermStructure(baseDate, frequency, dayCounter, seasonality, baseYoYRate) {}

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    const Date& referenceDate,
                                    Date baseDate,
                                    Rate baseYoYRate,
                                    Frequency frequency,
                                    const DayCounter& dayCounter,
                                    const ext::shared_ptr<Seasonality> &seasonality)
    : InflationTermStructure(referenceDate, baseDate, frequency, dayCounter, seasonality, baseYoYRate) {}

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    Date baseDate,
                                    Rate baseYoYRate,
                                    Frequency frequency,
                                    const DayCounter& dayCounter,
                                    const ext::shared_ptr<Seasonality> &seasonality)
    : InflationTermStructure(settlementDays, calendar, baseDate, frequency, dayCounter, seasonality, baseYoYRate) {}

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    Date baseDate,
                                    Rate baseYoYRate,
                                    Frequency frequency,
                                    bool indexIsInterpolated,
                                    const DayCounter& dayCounter,
                                    const ext::shared_ptr<Seasonality> &seasonality)
    : YoYInflationTermStructure(baseDate, baseYoYRate, frequency, dayCounter, seasonality) {
        indexIsInterpolated_ = indexIsInterpolated;
    }

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    const Date& referenceDate,
                                    Date baseDate,
                                    Rate baseYoYRate,
                                    Frequency frequency,
                                    bool indexIsInterpolated,
                                    const DayCounter& dayCounter,
                                    const ext::shared_ptr<Seasonality> &seasonality)
    : YoYInflationTermStructure(referenceDate, baseDate, baseYoYRate,
                                frequency, dayCounter, seasonality) {
        indexIsInterpolated_ = indexIsInterpolated;
    }

    YoYInflationTermStructure::YoYInflationTermStructure(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    Date baseDate,
                                    Rate baseYoYRate,
                                    Frequency frequency,
                                    bool indexIsInterpolated,
                                    const DayCounter& dayCounter,
                                    const ext::shared_ptr<Seasonality> &seasonality)
    : YoYInflationTermStructure(settlementDays, calendar, baseDate, baseYoYRate,
                                frequency, dayCounter, seasonality) {
        indexIsInterpolated_ = indexIsInterpolated;
    }

    QL_DEPRECATED_ENABLE_WARNING

    Rate YoYInflationTermStructure::yoyRate(const Date &d, const Period& instObsLag,
                                            bool forceLinearInterpolation,
                                            bool extrapolate) const {

        Period useLag = instObsLag;
        if (instObsLag == Period(-1,Days)) {
            useLag = Period(0, Days);
        }

        Rate yoyRate;
        if (forceLinearInterpolation) {
            std::pair<Date,Date> dd = inflationPeriod(d-useLag, frequency());
            dd.second = dd.second + Period(1,Days);
            Real dp = dd.second - dd.first;
            Real dt = (d-useLag) - dd.first;
            // if we are interpolating we only check the exact point
            // this prevents falling off the end at curve maturity
            InflationTermStructure::checkRange(d, extrapolate);
            Time t1 = timeFromReference(dd.first);
            Time t2 = timeFromReference(dd.second);
            Rate y1 = yoyRateImpl(t1);
            Rate y2 = yoyRateImpl(t2);
            yoyRate = y1 + (y2-y1) * (dt/dp);
        } else {
            QL_DEPRECATED_DISABLE_WARNING
            if (indexIsInterpolated()) {
                InflationTermStructure::checkRange(d-useLag, extrapolate);
                Time t = timeFromReference(d-useLag);
                yoyRate = yoyRateImpl(t);
            } else {
                std::pair<Date,Date> dd = inflationPeriod(d-useLag, frequency());
                InflationTermStructure::checkRange(dd.first, extrapolate);
                Time t = timeFromReference(dd.first);
                yoyRate = yoyRateImpl(t);
            }
            QL_DEPRECATED_ENABLE_WARNING
        }

        if (hasSeasonality()) {
            yoyRate = seasonality()->correctYoYRate(d-useLag, yoyRate, *this);
        }
        return yoyRate;
    }

    Rate YoYInflationTermStructure::yoyRate(Time t,
                                            bool extrapolate) const {
        checkRange(t, extrapolate);
        return yoyRateImpl(t);
    }




    std::pair<Date,Date> inflationPeriod(const Date& d,
                                         Frequency frequency) {
        Month month = d.month();
        Year year = d.year();

        Month startMonth, endMonth;
        switch (frequency) {
          case Annual:
          case Semiannual:
          case EveryFourthMonth:
          case Quarterly:
          case Bimonthly: {
                int nMonths = 12 / frequency;
                startMonth = Month(month - (month - 1) % nMonths);
                endMonth = Month(startMonth + nMonths - 1);
            }
            break;
          case Monthly:
            startMonth = endMonth = month;
            break;
          default:
            QL_FAIL("Frequency not handled: " << frequency);
            break;
        };

        return {Date(1, startMonth, year), Date::endOfMonth(Date(1, endMonth, year))};
    }


    Time inflationYearFraction(Frequency f, bool indexIsInterpolated,
                               const DayCounter &dayCounter,
                               const Date &d1, const Date &d2) {

        Time t=0;
        if (indexIsInterpolated) {
            // N.B. we do not use linear interpolation between flat
            // fixing forecasts for forecasts.  This avoids awkwardnesses
            // when bootstrapping the inflation curve.
            t = dayCounter.yearFraction(d1, d2);
        } else {
            // I.e. fixing is constant for the whole inflation period.
            // Use the value for half way along the period.
            // But the inflation time is the time between period starts
            std::pair<Date,Date> limD1 = inflationPeriod(d1, f);
            std::pair<Date,Date> limD2 = inflationPeriod(d2, f);
            t = dayCounter.yearFraction(limD1.first, limD2.first);
        }

        return t;
    }


}
