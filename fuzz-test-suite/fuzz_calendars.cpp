/*
 Copyright (C) 2026 OSS-Fuzz coverage pipeline

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the license
 for more details.
*/

//! Fuzz target: QuantLib calendars (ql/time/calendars/*).
//!
//! Threat model / rationale: QuantLib is a library embedded in pricing and
//! risk systems; market-data and trade feeds hand it dates and calendar
//! names (schedules, roll conventions, settlement calculations). The
//! per-country holiday logic in ql/time/calendars/*.cpp was 100% dark in
//! the 20261021 coverage report (4,205 dark lines over 105 files) because
//! no existing harness reaches Calendar at all. This harness selects one
//! of all available calendars from the fuzz input and drives the date
//! arithmetic an attacker-controlled feed can trigger: business-day
//! checks, holiday tables, adjustment and roll conventions, and
//! business-day counts. Calendar::addHoliday/removeHoliday are
//! deliberately NOT called: they mutate the shared Calendar::Impl state
//! and would leak state across iterations.

#include <fuzzer/FuzzedDataProvider.h>

#include <ql/time/calendar.hpp>
#include <ql/time/calendars/argentina.hpp>
#include <ql/time/calendars/australia.hpp>
#include <ql/time/calendars/austria.hpp>
#include <ql/time/calendars/bespokecalendar.hpp>
#include <ql/time/calendars/botswana.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/canada.hpp>
#include <ql/time/calendars/chile.hpp>
#include <ql/time/calendars/china.hpp>
#include <ql/time/calendars/croatia.hpp>
#include <ql/time/calendars/czechrepublic.hpp>
#include <ql/time/calendars/denmark.hpp>
#include <ql/time/calendars/finland.hpp>
#include <ql/time/calendars/france.hpp>
#include <ql/time/calendars/germany.hpp>
#include <ql/time/calendars/hongkong.hpp>
#include <ql/time/calendars/hungary.hpp>
#include <ql/time/calendars/iceland.hpp>
#include <ql/time/calendars/india.hpp>
#include <ql/time/calendars/indonesia.hpp>
#include <ql/time/calendars/israel.hpp>
#include <ql/time/calendars/italy.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/malaysia.hpp>
#include <ql/time/calendars/malta.hpp>
#include <ql/time/calendars/mexico.hpp>
#include <ql/time/calendars/montenegro.hpp>
#include <ql/time/calendars/newzealand.hpp>
#include <ql/time/calendars/northmacedonia.hpp>
#include <ql/time/calendars/norway.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/philippines.hpp>
#include <ql/time/calendars/poland.hpp>
#include <ql/time/calendars/romania.hpp>
#include <ql/time/calendars/russia.hpp>
#include <ql/time/calendars/saudiarabia.hpp>
#include <ql/time/calendars/serbia.hpp>
#include <ql/time/calendars/singapore.hpp>
#include <ql/time/calendars/slovakia.hpp>
#include <ql/time/calendars/slovenia.hpp>
#include <ql/time/calendars/southafrica.hpp>
#include <ql/time/calendars/southkorea.hpp>
#include <ql/time/calendars/sweden.hpp>
#include <ql/time/calendars/switzerland.hpp>
#include <ql/time/calendars/taiwan.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/thailand.hpp>
#include <ql/time/calendars/turkey.hpp>
#include <ql/time/calendars/ukraine.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/uzbekistan.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/date.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/timeunit.hpp>
#include <algorithm>

using namespace QuantLib;

namespace {

constexpr int kNumCalendars = 54;

Calendar makeCalendar(int which) {
    switch (which) {
    case 0: return Argentina();
    case 1: return Australia();
    case 2: return Austria();
    case 3: return BespokeCalendar();
    case 4: return Botswana();
    case 5: return Brazil();
    case 6: return Canada();
    case 7: return Chile();
    case 8: return China();
    case 9: return Croatia();
    case 10: return CzechRepublic();
    case 11: return Denmark();
    case 12: return Finland();
    case 13: return France();
    case 14: return Germany();
    case 15: return HongKong();
    case 16: return Hungary();
    case 17: return Iceland();
    case 18: return India();
    case 19: return Indonesia();
    case 20: return Israel();
    case 21: return Italy();
    case 22: return Japan();
    case 23: return JointCalendar(TARGET(), UnitedStates(UnitedStates::Settlement));
    case 24: return Malaysia();
    case 25: return Malta();
    case 26: return Mexico();
    case 27: return Montenegro();
    case 28: return NewZealand();
    case 29: return NorthMacedonia();
    case 30: return Norway();
    case 31: return NullCalendar();
    case 32: return Philippines();
    case 33: return Poland();
    case 34: return Romania();
    case 35: return Russia();
    case 36: return SaudiArabia();
    case 37: return Serbia();
    case 38: return Singapore();
    case 39: return Slovakia();
    case 40: return Slovenia();
    case 41: return SouthAfrica();
    case 42: return SouthKorea();
    case 43: return Sweden();
    case 44: return Switzerland();
    case 45: return Taiwan();
    case 46: return TARGET();
    case 47: return Thailand();
    case 48: return Turkey();
    case 49: return Ukraine();
    case 50: return UnitedKingdom();
    case 51: return UnitedStates(UnitedStates::Settlement);
    case 52: return Uzbekistan();
    default: return WeekendsOnly();
    }
}

BusinessDayConvention pickConvention(int c) {
    switch (c % 7) {
    case 0: return Following;
    case 1: return ModifiedFollowing;
    case 2: return HalfMonthModifiedFollowing;
    case 3: return Nearest;
    case 4: return Preceding;
    case 5: return ModifiedPreceding;
    default: return Unadjusted;
    }
}

TimeUnit pickUnit(int u) {
    switch (u % 4) {
    case 0: return Days;
    case 1: return Weeks;
    case 2: return Months;
    default: return Years;
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);

    try {
        const Calendar cal = makeCalendar(
            fdp.ConsumeIntegralInRange<int>(0, kNumCalendars - 1));

        // Two dates anywhere inside QuantLib's supported range
        // (1901-01-01 .. 2199-12-31): feeds can carry extreme dates.
        const auto s1 = static_cast<Date::serial_type>(
            fdp.ConsumeIntegralInRange<int>(
                static_cast<int>(Date::minDate().serialNumber()),
                static_cast<int>(Date::maxDate().serialNumber())));
        const auto s2 = static_cast<Date::serial_type>(
            fdp.ConsumeIntegralInRange<int>(
                static_cast<int>(Date::minDate().serialNumber()),
                static_cast<int>(Date::maxDate().serialNumber())));
        const Date d1(s1), d2(s2);

        const BusinessDayConvention conv = pickConvention(
            fdp.ConsumeIntegralInRange<int>(0, 6));

        // Business-day queries: walk the per-country holiday tables.
        (void)cal.isBusinessDay(d1);
        (void)cal.isHoliday(d1);
        (void)cal.isWeekend(d1.weekday());
        (void)cal.isEndOfMonth(d1);
        (void)cal.endOfMonth(d1);

        // Roll conventions over arbitrary dates.
        (void)cal.adjust(d1, conv);
        (void)cal.adjust(d2, conv);

        // Date arithmetic with a fuzzed offset.
        const Integer n =
            static_cast<Integer>(fdp.ConsumeIntegralInRange<int>(-400, 400));
        const TimeUnit unit = pickUnit(fdp.ConsumeIntegralInRange<int>(0, 3));
        const bool eom = fdp.ConsumeBool();
        (void)cal.advance(d1, n, unit, conv, eom);
        (void)cal.advance(d2, n, unit, conv, eom);

        // Business-day counting over a fuzzed interval.
        (void)cal.businessDaysBetween(d1, d2, true, true);
        (void)cal.businessDaysBetween(d2, d1, false, false);

        // Holiday listing (holidayList requires from <= to). Bound the
        // span so the listing stays fast even for the full date range.
        const Date from = std::min(d1, d2);
        const Date to = std::max(d1, d2);
        if (to - from <= 3700) {
            (void)cal.holidayList(from, to, false);
            (void)cal.holidayList(from, to, true);
        }
    } catch (const std::exception&) {
        // QuantLib reports invalid input via exceptions; not a finding.
    }
    return 0;
}

