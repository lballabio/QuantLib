"""Tests for foundational enums: TimeUnit, Frequency, Weekday, Month.

Transpiled from C++ enum definitions in:
  - upstream/ql/time/timeunit.hpp
  - upstream/ql/time/frequency.hpp
  - upstream/ql/time/weekday.hpp
  - upstream/ql/time/date.hpp (Month)
"""

from quantlib.time.frequency import Frequency
from quantlib.time.month import Month
from quantlib.time.timeunit import TimeUnit
from quantlib.time.weekday import Weekday


class TestTimeUnit:
    def test_values(self):
        assert TimeUnit.Days == 0
        assert TimeUnit.Weeks == 1
        assert TimeUnit.Months == 2
        assert TimeUnit.Years == 3
        assert TimeUnit.Hours == 4
        assert TimeUnit.Minutes == 5
        assert TimeUnit.Seconds == 6
        assert TimeUnit.Milliseconds == 7
        assert TimeUnit.Microseconds == 8

    def test_member_count(self):
        assert len(TimeUnit) == 9

    def test_is_int(self):
        assert isinstance(TimeUnit.Days, int)


class TestFrequency:
    def test_values(self):
        assert Frequency.NoFrequency == -1
        assert Frequency.Once == 0
        assert Frequency.Annual == 1
        assert Frequency.Semiannual == 2
        assert Frequency.EveryFourthMonth == 3
        assert Frequency.Quarterly == 4
        assert Frequency.Bimonthly == 6
        assert Frequency.Monthly == 12
        assert Frequency.EveryFourthWeek == 13
        assert Frequency.Biweekly == 26
        assert Frequency.Weekly == 52
        assert Frequency.Daily == 365
        assert Frequency.OtherFrequency == 999

    def test_member_count(self):
        assert len(Frequency) == 13

    def test_is_int(self):
        assert isinstance(Frequency.Annual, int)


class TestWeekday:
    def test_values(self):
        assert Weekday.Sunday == 1
        assert Weekday.Monday == 2
        assert Weekday.Tuesday == 3
        assert Weekday.Wednesday == 4
        assert Weekday.Thursday == 5
        assert Weekday.Friday == 6
        assert Weekday.Saturday == 7

    def test_short_aliases(self):
        assert Weekday.Sun == Weekday.Sunday
        assert Weekday.Mon == Weekday.Monday
        assert Weekday.Tue == Weekday.Tuesday
        assert Weekday.Wed == Weekday.Wednesday
        assert Weekday.Thu == Weekday.Thursday
        assert Weekday.Fri == Weekday.Friday
        assert Weekday.Sat == Weekday.Saturday

    def test_member_count(self):
        # 7 unique values; aliases don't add members
        assert len(Weekday) == 7

    def test_is_int(self):
        assert isinstance(Weekday.Monday, int)


class TestMonth:
    def test_values(self):
        assert Month.January == 1
        assert Month.February == 2
        assert Month.March == 3
        assert Month.April == 4
        assert Month.May == 5
        assert Month.June == 6
        assert Month.July == 7
        assert Month.August == 8
        assert Month.September == 9
        assert Month.October == 10
        assert Month.November == 11
        assert Month.December == 12

    def test_short_aliases(self):
        assert Month.Jan == Month.January
        assert Month.Feb == Month.February
        assert Month.Mar == Month.March
        assert Month.Apr == Month.April
        assert Month.Jun == Month.June
        assert Month.Jul == Month.July
        assert Month.Aug == Month.August
        assert Month.Sep == Month.September
        assert Month.Oct == Month.October
        assert Month.Nov == Month.November
        assert Month.Dec == Month.December

    def test_member_count(self):
        # 12 unique values; aliases don't add members
        assert len(Month) == 12

    def test_is_int(self):
        assert isinstance(Month.January, int)
