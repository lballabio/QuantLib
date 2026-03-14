"""Tests for Calendar base class and calendar implementations.

Transpiled from upstream/test-suite/calendars.cpp.
"""

import pytest

from quantlib.time.businessdayconvention import BusinessDayConvention
from quantlib.time.calendar import Calendar
from quantlib.time.calendars.nullcalendar import NullCalendar
from quantlib.time.calendars.target import TARGET
from quantlib.time.calendars.unitedkingdom import UnitedKingdom
from quantlib.time.calendars.unitedstates import UnitedStates
from quantlib.time.calendars.weekendsonly import WeekendsOnly
from quantlib.time.date import Date
from quantlib.time.month import Month
from quantlib.time.period import Period
from quantlib.time.timeunit import TimeUnit
from quantlib.time.weekday import Weekday


class TestCalendarBase:
    def test_empty_calendar(self):
        c = Calendar()
        assert c.empty()

    def test_empty_raises(self):
        with pytest.raises(RuntimeError):
            Calendar().name()

    def test_equality(self):
        c1 = TARGET()
        c2 = TARGET()
        assert c1 == c2

    def test_inequality(self):
        c1 = TARGET()
        c2 = NullCalendar()
        assert c1 != c2


class TestBusinessDayConvention:
    def test_enum_values(self):
        assert BusinessDayConvention.Following == 0
        assert BusinessDayConvention.ModifiedFollowing == 1
        assert BusinessDayConvention.Preceding == 2
        assert BusinessDayConvention.Unadjusted == 4
        assert BusinessDayConvention.Nearest == 6


class TestNullCalendar:
    def test_name(self):
        assert NullCalendar().name() == "Null"

    def test_every_day_is_business_day(self):
        cal = NullCalendar()
        # Saturday
        d = Date(4, Month.January, 2020)
        assert d.weekday() == Weekday.Saturday
        assert cal.isBusinessDay(d)

    def test_no_weekends(self):
        cal = NullCalendar()
        assert not cal.isWeekend(Weekday.Saturday)


class TestWeekendsOnly:
    def test_name(self):
        assert WeekendsOnly().name() == "weekends only"

    def test_weekday_is_business_day(self):
        cal = WeekendsOnly()
        d = Date(6, Month.January, 2020)  # Monday
        assert cal.isBusinessDay(d)

    def test_weekend_is_holiday(self):
        cal = WeekendsOnly()
        d = Date(4, Month.January, 2020)  # Saturday
        assert cal.isHoliday(d)


class TestTARGET:
    def test_name(self):
        assert TARGET().name() == "TARGET"

    def test_new_years_day(self):
        cal = TARGET()
        assert cal.isHoliday(Date(1, Month.January, 2020))

    def test_christmas(self):
        cal = TARGET()
        assert cal.isHoliday(Date(25, Month.December, 2020))

    def test_good_friday_2020(self):
        cal = TARGET()
        # Good Friday 2020 = April 10
        assert cal.isHoliday(Date(10, Month.April, 2020))

    def test_easter_monday_2020(self):
        cal = TARGET()
        # Easter Monday 2020 = April 13
        assert cal.isHoliday(Date(13, Month.April, 2020))

    def test_labour_day(self):
        cal = TARGET()
        assert cal.isHoliday(Date(1, Month.May, 2020))

    def test_regular_business_day(self):
        cal = TARGET()
        assert cal.isBusinessDay(Date(2, Month.January, 2020))  # Thursday


class TestUnitedStatesSettlement:
    def test_name(self):
        assert UnitedStates(UnitedStates.Market.Settlement).name() == "US settlement"

    def test_new_years_day(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        assert cal.isHoliday(Date(1, Month.January, 2020))

    def test_mlk_day_2020(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        # Third Monday in January 2020 = Jan 20
        assert cal.isHoliday(Date(20, Month.January, 2020))

    def test_presidents_day_2020(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        # Third Monday in February 2020 = Feb 17
        assert cal.isHoliday(Date(17, Month.February, 2020))

    def test_memorial_day_2020(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        # Last Monday in May 2020 = May 25
        assert cal.isHoliday(Date(25, Month.May, 2020))

    def test_independence_day(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        # July 4, 2020 is Saturday → observed Friday July 3
        assert cal.isHoliday(Date(3, Month.July, 2020))

    def test_labor_day_2020(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        # First Monday in September 2020 = Sep 7
        assert cal.isHoliday(Date(7, Month.September, 2020))

    def test_thanksgiving_2020(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        # Fourth Thursday in November 2020 = Nov 26
        assert cal.isHoliday(Date(26, Month.November, 2020))

    def test_christmas_2020(self):
        cal = UnitedStates(UnitedStates.Market.Settlement)
        assert cal.isHoliday(Date(25, Month.December, 2020))


class TestUnitedStatesNYSE:
    def test_name(self):
        assert UnitedStates(UnitedStates.Market.NYSE).name() == "New York stock exchange"

    def test_good_friday_2020(self):
        cal = UnitedStates(UnitedStates.Market.NYSE)
        assert cal.isHoliday(Date(10, Month.April, 2020))


class TestUnitedKingdom:
    def test_name(self):
        assert UnitedKingdom().name() == "UK settlement"

    def test_new_years_day(self):
        cal = UnitedKingdom()
        assert cal.isHoliday(Date(1, Month.January, 2020))

    def test_christmas(self):
        cal = UnitedKingdom()
        assert cal.isHoliday(Date(25, Month.December, 2020))


class TestCalendarAdjust:
    def test_following(self):
        cal = TARGET()
        # Jan 1 2020 (holiday) → Jan 2 (Thursday)
        d = cal.adjust(Date(1, Month.January, 2020), BusinessDayConvention.Following)
        assert d == Date(2, Month.January, 2020)

    def test_preceding(self):
        cal = TARGET()
        # Jan 1 2020 (holiday) → Dec 31 2019 (Tuesday)
        d = cal.adjust(Date(1, Month.January, 2020), BusinessDayConvention.Preceding)
        assert d == Date(31, Month.December, 2019)

    def test_unadjusted(self):
        cal = TARGET()
        d = cal.adjust(Date(1, Month.January, 2020), BusinessDayConvention.Unadjusted)
        assert d == Date(1, Month.January, 2020)

    def test_modified_following(self):
        cal = TARGET()
        # A holiday at month end should roll backward
        # March 29 2024 is Good Friday, March 30-31 weekend
        d = cal.adjust(Date(29, Month.March, 2024), BusinessDayConvention.ModifiedFollowing)
        assert d == Date(28, Month.March, 2024)


class TestCalendarAdvance:
    def test_advance_days(self):
        cal = TARGET()
        d = Date(2, Month.January, 2020)  # Thursday
        # Advance 2 business days
        result = cal.advance(d, 2, TimeUnit.Days)
        assert result == Date(6, Month.January, 2020)  # Monday (skips weekend)

    def test_advance_months(self):
        cal = TARGET()
        d = Date(31, Month.January, 2020)
        result = cal.advance(d, 1, TimeUnit.Months)
        # Jan 31 + 1M = Feb 29 (clamped), Saturday → Following = Mar 2
        assert result == Date(2, Month.March, 2020)

    def test_advance_with_period(self):
        cal = TARGET()
        d = Date(2, Month.January, 2020)
        result = cal.advance(d, period=Period(1, TimeUnit.Months))
        # Feb 2 is Sunday → Feb 3
        assert result == Date(3, Month.February, 2020)


class TestBusinessDaysBetween:
    def test_same_date(self):
        cal = TARGET()
        d = Date(2, Month.January, 2020)
        assert cal.businessDaysBetween(d, d) == 0

    def test_one_week(self):
        cal = WeekendsOnly()
        d1 = Date(6, Month.January, 2020)  # Monday
        d2 = Date(10, Month.January, 2020)  # Friday
        assert cal.businessDaysBetween(d1, d2) == 4  # Mon-Thu (excludeLast default)
        assert cal.businessDaysBetween(d1, d2, includeFirst=True, includeLast=True) == 5


class TestAddRemoveHoliday:
    def test_add_holiday(self):
        cal = WeekendsOnly()
        d = Date(6, Month.January, 2020)  # Monday
        assert cal.isBusinessDay(d)
        cal.addHoliday(d)
        assert cal.isHoliday(d)

    def test_remove_holiday(self):
        cal = TARGET()
        d = Date(1, Month.January, 2020)  # New Year
        assert cal.isHoliday(d)
        cal.removeHoliday(d)
        assert cal.isBusinessDay(d)

    def test_holiday_list(self):
        cal = TARGET()
        holidays = cal.holidayList(Date(1, Month.January, 2020), Date(31, Month.January, 2020))
        assert Date(1, Month.January, 2020) in holidays

    def test_business_day_list(self):
        cal = WeekendsOnly()
        days = cal.businessDayList(Date(6, Month.January, 2020), Date(10, Month.January, 2020))
        assert len(days) == 5  # Mon-Fri


class TestEndOfMonth:
    def test_is_end_of_month(self):
        cal = TARGET()
        # Dec 31 2019 is Tuesday and last business day
        assert cal.isEndOfMonth(Date(31, Month.December, 2019))

    def test_end_of_month(self):
        cal = TARGET()
        d = Date(15, Month.February, 2020)
        eom = cal.endOfMonth(d)
        # Feb 29 2020 is Saturday → Feb 28 (Friday)
        assert eom == Date(28, Month.February, 2020)
