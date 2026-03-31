/*
 Copyright (C) 2026 David Korczynski

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

#include <ql/time/asx.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/argentina.hpp>
#include <ql/time/calendars/australia.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/canada.hpp>
#include <ql/time/calendars/china.hpp>
#include <ql/time/calendars/germany.hpp>
#include <ql/time/calendars/india.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/southkorea.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/ecb.hpp>
#include <ql/time/imm.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);

    try {
        // --- Calendar operations with fuzz-selected calendar ---
        Calendar calendars[] = {
            TARGET(),
            UnitedStates(UnitedStates::GovernmentBond),
            UnitedStates(UnitedStates::NYSE),
            UnitedStates(UnitedStates::FederalReserve),
            UnitedKingdom(),
            Germany(Germany::Eurex),
            Germany(Germany::FrankfurtStockExchange),
            Japan(),
            China(China::SSE),
            Canada(),
            Australia(),
            Brazil(),
            India(),
            SouthKorea(SouthKorea::KRX),
            Argentina(),
        };
        auto calIdx = fdp.ConsumeIntegralInRange<int>(0, 14);
        Calendar cal = calendars[calIdx];

        // Build dates from fuzz input
        auto year1 = fdp.ConsumeIntegralInRange<int>(1901, 2199);
        auto month1 = fdp.ConsumeIntegralInRange<int>(1, 12);
        auto day1 = fdp.ConsumeIntegralInRange<int>(1, 28);
        auto year2 = fdp.ConsumeIntegralInRange<int>(1901, 2199);
        auto month2 = fdp.ConsumeIntegralInRange<int>(1, 12);
        auto day2 = fdp.ConsumeIntegralInRange<int>(1, 28);

        Date d1(day1, static_cast<Month>(month1), year1);
        Date d2(day2, static_cast<Month>(month2), year2);

        // Exercise calendar functions
        (void)cal.isBusinessDay(d1);
        (void)cal.isHoliday(d1);
        (void)cal.isEndOfMonth(d1);
        (void)cal.endOfMonth(d1);
        (void)cal.name();

        BusinessDayConvention convs[] = {
            Following, ModifiedFollowing, Preceding,
            ModifiedPreceding, Unadjusted, Nearest,
            HalfMonthModifiedFollowing};
        auto convIdx = fdp.ConsumeIntegralInRange<int>(0, 6);
        BusinessDayConvention conv = convs[convIdx];

        (void)cal.adjust(d1, conv);

        auto advanceDays = fdp.ConsumeIntegralInRange<int>(-365, 365);
        (void)cal.advance(d1, advanceDays, Days, conv);

        auto advanceMonths = fdp.ConsumeIntegralInRange<int>(-24, 24);
        (void)cal.advance(d1, advanceMonths, Months, conv, true);

        if (d1 < d2) {
            (void)cal.businessDaysBetween(d1, d2);
            (void)cal.holidayList(d1, d2);
        }

        // --- Day counter operations ---
        DayCounter dayCounters[] = {
            Actual360(), Actual365Fixed(),
            Thirty360(Thirty360::BondBasis),
            Thirty360(Thirty360::EurobondBasis),
            ActualActual(ActualActual::ISMA),
            ActualActual(ActualActual::ISDA),
            ActualActual(ActualActual::AFB),
            Business252(cal)};
        auto dcIdx = fdp.ConsumeIntegralInRange<int>(0, 7);
        DayCounter dc = dayCounters[dcIdx];

        if (d1 < d2) {
            (void)dc.dayCount(d1, d2);
            (void)dc.yearFraction(d1, d2);
        }

        // --- IMM / ASX / ECB date functions ---
        (void)IMM::isIMMdate(d1);
        (void)IMM::nextDate(d1);
        (void)ASX::isASXdate(d1);
        (void)ASX::nextDate(d1);
        (void)ECB::nextDate(d1);

        // --- Schedule generation ---
        Frequency freqs[] = {Annual, Semiannual, Quarterly,
                             Monthly, Weekly, Biweekly};
        auto freqIdx = fdp.ConsumeIntegralInRange<int>(0, 5);
        Frequency freq = freqs[freqIdx];

        DateGeneration::Rule rules[] = {
            DateGeneration::Backward, DateGeneration::Forward,
            DateGeneration::Zero, DateGeneration::ThirdWednesday,
            DateGeneration::Twentieth};
        auto ruleIdx = fdp.ConsumeIntegralInRange<int>(0, 4);
        DateGeneration::Rule rule = rules[ruleIdx];

        if (d1 < d2) {
            Date start = d1;
            Date end = d2;
            // Cap the range to avoid extremely long schedules
            if (end - start > 365 * 40)
                end = start + 365 * 40;

            Schedule schedule(start, end, Period(freq), cal,
                              conv, conv, rule, false);
            (void)schedule.size();
            (void)schedule.dates();
            if (schedule.size() > 0) {
                (void)schedule.startDate();
                (void)schedule.endDate();
            }
        }

        // --- Period parsing from fuzz strings ---
        auto periodStr = fdp.ConsumeRandomLengthString(20);
        (void)PeriodParser::parse(periodStr);

        // --- Date parsing from fuzz strings ---
        auto dateStr = fdp.ConsumeRandomLengthString(30);
        (void)DateParser::parseISO(dateStr);
        (void)DateParser::parseFormatted(dateStr, "%Y-%m-%d");

    } catch (const std::exception&) {
    }
    return 0;
}
