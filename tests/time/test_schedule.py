"""Tests for Schedule and MakeSchedule.

Transpiled from upstream/test-suite/schedule.cpp.
"""

from quantlib.time.businessdayconvention import BusinessDayConvention
from quantlib.time.calendars.nullcalendar import NullCalendar
from quantlib.time.calendars.target import TARGET
from quantlib.time.date import Date
from quantlib.time.dategenerationrule import DateGeneration
from quantlib.time.frequency import Frequency
from quantlib.time.month import Month
from quantlib.time.period import Period
from quantlib.time.schedule import MakeSchedule, Schedule
from quantlib.time.timeunit import TimeUnit


class TestScheduleBasic:
    def test_empty_schedule(self):
        s = Schedule()
        assert s.empty()
        assert s.size() == 0

    def test_list_constructor(self):
        dates = [
            Date(1, Month.January, 2020),
            Date(1, Month.April, 2020),
            Date(1, Month.July, 2020),
        ]
        s = Schedule(dates=dates)
        assert s.size() == 3
        assert s.startDate() == dates[0]
        assert s.endDate() == dates[2]

    def test_date_access(self):
        dates = [
            Date(1, Month.January, 2020),
            Date(1, Month.July, 2020),
        ]
        s = Schedule(dates=dates)
        assert s[0] == dates[0]
        assert s[1] == dates[1]
        assert s.date(0) == dates[0]


class TestBackwardGeneration:
    def test_quarterly_backward(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.January, 2021),
            tenor=Period(3, TimeUnit.Months),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Backward,
            endOfMonth=False,
        )
        expected = [
            Date(1, Month.January, 2020),
            Date(1, Month.April, 2020),
            Date(1, Month.July, 2020),
            Date(1, Month.October, 2020),
            Date(1, Month.January, 2021),
        ]
        assert s.size() == len(expected)
        for i, d in enumerate(expected):
            assert s[i] == d, f"date {i}: {s[i]} != {d}"

    def test_semiannual_backward(self):
        s = Schedule(
            effectiveDate=Date(15, Month.March, 2020),
            terminationDate=Date(15, Month.March, 2022),
            tenor=Period(6, TimeUnit.Months),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Backward,
            endOfMonth=False,
        )
        assert s.size() == 5  # 4 periods + 1
        assert s.startDate() == Date(15, Month.March, 2020)
        assert s.endDate() == Date(15, Month.March, 2022)


class TestForwardGeneration:
    def test_quarterly_forward(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.January, 2021),
            tenor=Period(3, TimeUnit.Months),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Forward,
            endOfMonth=False,
        )
        expected = [
            Date(1, Month.January, 2020),
            Date(1, Month.April, 2020),
            Date(1, Month.July, 2020),
            Date(1, Month.October, 2020),
            Date(1, Month.January, 2021),
        ]
        assert s.size() == len(expected)
        for i, d in enumerate(expected):
            assert s[i] == d, f"date {i}: {s[i]} != {d}"

    def test_monthly_forward_with_target(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.April, 2020),
            tenor=Period(1, TimeUnit.Months),
            calendar=TARGET(),
            convention=BusinessDayConvention.Following,
            terminationDateConvention=BusinessDayConvention.Following,
            rule=DateGeneration.Rule.Forward,
            endOfMonth=False,
        )
        # Jan 1 is TARGET holiday → adjusted to Jan 2
        assert s[0] == Date(2, Month.January, 2020)
        assert s.size() == 4


class TestZeroGeneration:
    def test_zero_rule(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.July, 2020),
            tenor=Period(0, TimeUnit.Years),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Zero,
            endOfMonth=False,
        )
        assert s.size() == 2
        assert s[0] == Date(1, Month.January, 2020)
        assert s[1] == Date(1, Month.July, 2020)


class TestMakeSchedule:
    def test_basic_build(self):
        s = (
            MakeSchedule()
            .set_from(Date(1, Month.January, 2020))
            .to(Date(1, Month.January, 2021))
            .withTenor(Period(3, TimeUnit.Months))
            .backwards()
            .build()
        )
        assert s.size() == 5

    def test_with_frequency(self):
        s = (
            MakeSchedule()
            .set_from(Date(1, Month.January, 2020))
            .to(Date(1, Month.January, 2021))
            .withFrequency(Frequency.Quarterly)
            .backwards()
            .build()
        )
        assert s.size() == 5

    def test_with_calendar_and_convention(self):
        s = (
            MakeSchedule()
            .set_from(Date(1, Month.January, 2020))
            .to(Date(1, Month.April, 2020))
            .withTenor(Period(1, TimeUnit.Months))
            .withCalendar(TARGET())
            .withConvention(BusinessDayConvention.Following)
            .forwards()
            .build()
        )
        # Jan 1 is TARGET holiday
        assert s[0] == Date(2, Month.January, 2020)

    def test_daily_schedule(self):
        """Transpiled from testDailySchedule in schedule.cpp."""
        start = Date(17, Month.January, 2012)
        s = (
            MakeSchedule()
            .set_from(start)
            .to(start + 7)
            .withCalendar(TARGET())
            .withFrequency(Frequency.Daily)
            .withConvention(BusinessDayConvention.Preceding)
            .build()
        )
        expected = [
            Date(17, Month.January, 2012),
            Date(18, Month.January, 2012),
            Date(19, Month.January, 2012),
            Date(20, Month.January, 2012),
            Date(23, Month.January, 2012),
            Date(24, Month.January, 2012),
        ]
        assert s.size() == len(expected), f"expected {len(expected)} dates, got {s.size()}"
        for i, d in enumerate(expected):
            assert s[i] == d, f"expected {d} at index {i}, found {s[i]}"


class TestScheduleInspectors:
    def test_tenor(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.January, 2021),
            tenor=Period(3, TimeUnit.Months),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Backward,
            endOfMonth=False,
        )
        assert s.hasTenor()
        assert s.tenor() == Period(3, TimeUnit.Months)

    def test_is_regular(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.January, 2021),
            tenor=Period(3, TimeUnit.Months),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Backward,
            endOfMonth=False,
        )
        assert s.hasIsRegular()
        # All periods should be regular for aligned dates
        for i in range(1, s.size()):
            assert s.isRegular(i), f"period {i} should be regular"

    def test_iteration(self):
        s = Schedule(
            effectiveDate=Date(1, Month.January, 2020),
            terminationDate=Date(1, Month.July, 2020),
            tenor=Period(3, TimeUnit.Months),
            calendar=NullCalendar(),
            convention=BusinessDayConvention.Unadjusted,
            terminationDateConvention=BusinessDayConvention.Unadjusted,
            rule=DateGeneration.Rule.Forward,
            endOfMonth=False,
        )
        dates = list(s)
        assert len(dates) == 3
        assert dates[0] == Date(1, Month.January, 2020)
