"""Tests for Date class — constructors and inspectors.

Transpiled from upstream/test-suite/dates.cpp (testConsistency, null date).
"""

from quantlib.time.date import Date
from quantlib.time.month import Month
from quantlib.time.weekday import Weekday


class TestNullDate:
    def test_default_constructor(self):
        d = Date()
        assert d.serialNumber() == 0

    def test_null_date_is_falsy(self):
        assert not Date()

    def test_null_date_str(self):
        assert str(Date()) == "null date"


class TestConstructors:
    def test_serial_number_constructor(self):
        d = Date(367)  # Jan 1, 1901
        assert d.serialNumber() == 367

    def test_dmy_constructor(self):
        d = Date(1, Month.January, 1901)
        assert d.serialNumber() == 367

    def test_serial_and_dmy_agree(self):
        d1 = Date(367)
        d2 = Date(1, Month.January, 1901)
        assert d1 == d2

    def test_known_dates(self):
        # Dec 31, 2199 is the max date
        d = Date(31, Month.December, 2199)
        assert d.serialNumber() == 109574

        # Jan 1, 1901 is the min date
        d = Date(1, Month.January, 1901)
        assert d.serialNumber() == 367


class TestInspectors:
    def test_weekday(self):
        # January 15, 2002 was a Tuesday
        d = Date(15, Month.January, 2002)
        assert d.weekday() == Weekday.Tuesday

    def test_day_of_month(self):
        d = Date(15, Month.January, 2002)
        assert d.dayOfMonth() == 15

    def test_month(self):
        d = Date(15, Month.January, 2002)
        assert d.month() == Month.January

    def test_year(self):
        d = Date(15, Month.January, 2002)
        assert d.year() == 2002

    def test_day_of_year(self):
        d = Date(15, Month.January, 2002)
        assert d.dayOfYear() == 15

        # March 1 in non-leap year
        d = Date(1, Month.March, 2002)
        assert d.dayOfYear() == 31 + 28 + 1  # 60

    def test_day_of_year_leap(self):
        d = Date(1, Month.March, 2004)
        assert d.dayOfYear() == 31 + 29 + 1  # 61


class TestMinMaxDate:
    def test_min_date(self):
        d = Date.minDate()
        assert d.dayOfMonth() == 1
        assert d.month() == Month.January
        assert d.year() == 1901

    def test_max_date(self):
        d = Date.maxDate()
        assert d.dayOfMonth() == 31
        assert d.month() == Month.December
        assert d.year() == 2199


class TestIsLeap:
    def test_known_leap_years(self):
        assert Date.isLeap(2000)
        assert Date.isLeap(2004)
        assert Date.isLeap(2024)

    def test_known_non_leap_years(self):
        assert not Date.isLeap(1901)
        assert not Date.isLeap(2100)  # divisible by 100 but not 400
        assert not Date.isLeap(2001)

    def test_2100_is_not_leap(self):
        assert not Date.isLeap(2100)


class TestConsistency:
    """Full consistency check over entire date range, transpiled from C++."""

    def test_consistency(self):
        min_serial = Date.minDate().serialNumber() + 1
        max_serial = Date.maxDate().serialNumber()

        prev = Date(min_serial - 1)
        dy_old = prev.dayOfYear()
        d_old = prev.dayOfMonth()
        m_old = int(prev.month())
        y_old = prev.year()
        wd_old = int(prev.weekday())

        for i in range(min_serial, max_serial + 1):
            t = Date(i)
            assert t.serialNumber() == i, f"serial mismatch for {t}"

            dy = t.dayOfYear()
            d = t.dayOfMonth()
            m = int(t.month())
            y = t.year()
            wd = int(t.weekday())

            # day of year increments correctly
            assert (
                dy == dy_old + 1
                or (dy == 1 and dy_old == 365 and not Date.isLeap(y_old))
                or (dy == 1 and dy_old == 366 and Date.isLeap(y_old))
            ), f"wrong dayOfYear increment at {t}"

            # day/month/year increment correctly
            assert (
                (d == d_old + 1 and m == m_old and y == y_old)
                or (d == 1 and m == m_old + 1 and y == y_old)
                or (d == 1 and m == 1 and y == y_old + 1)
            ), f"wrong d/m/y increment at {t}"

            # valid month
            assert 1 <= m <= 12, f"invalid month at {t}"

            # valid day
            assert d >= 1, f"invalid day at {t}"

            # weekday increments correctly
            assert (wd == wd_old + 1) or (wd == 1 and wd_old == 7), f"wrong weekday at {t}"

            # dmy constructor roundtrip
            s = Date(d, Month(m), y)
            assert s.serialNumber() == i, f"dmy roundtrip failed at {t}"

            dy_old, d_old, m_old, y_old, wd_old = dy, d, m, y, wd


class TestComparison:
    def test_equality(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.January, 2020)
        assert d1 == d2

    def test_inequality(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(2, Month.January, 2020)
        assert d1 != d2

    def test_ordering(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(2, Month.January, 2020)
        assert d1 < d2
        assert d2 > d1
        assert d1 <= d2
        assert d2 >= d1

    def test_hashable(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.January, 2020)
        s = {d1, d2}
        assert len(s) == 1
