"""Tests for day count conventions.

Transpiled from upstream/test-suite/daycounters.cpp.
"""

import pytest

from quantlib.time.date import Date
from quantlib.time.daycounter import DayCounter
from quantlib.time.daycounters.actual360 import Actual360
from quantlib.time.daycounters.actual364 import Actual364
from quantlib.time.daycounters.actual365fixed import Actual365Fixed
from quantlib.time.daycounters.actual366 import Actual366
from quantlib.time.daycounters.actual36525 import Actual36525
from quantlib.time.daycounters.actualactual import ActualActual
from quantlib.time.daycounters.business252 import Business252
from quantlib.time.daycounters.one import OneDayCounter
from quantlib.time.daycounters.simpledaycounter import SimpleDayCounter
from quantlib.time.daycounters.thirty360 import Thirty360
from quantlib.time.daycounters.thirty365 import Thirty365
from quantlib.time.month import Month


class TestDayCounterBase:
    def test_empty_day_counter(self):
        dc = DayCounter()
        assert dc.empty()

    def test_empty_raises_on_name(self):
        with pytest.raises(RuntimeError):
            DayCounter().name()

    def test_equality(self):
        dc1 = Actual360()
        dc2 = Actual360()
        assert dc1 == dc2

    def test_inequality(self):
        dc1 = Actual360()
        dc2 = Actual365Fixed()
        assert dc1 != dc2


class TestActual360:
    def test_name(self):
        assert Actual360().name() == "Actual/360"
        assert Actual360(includeLastDay=True).name() == "Actual/360 (inc)"

    def test_day_count(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        dc = Actual360()
        assert dc.dayCount(d1, d2) == 182

    def test_year_fraction(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        dc = Actual360()
        assert dc.yearFraction(d1, d2) == pytest.approx(182.0 / 360.0)

    def test_include_last_day(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        dc = Actual360(includeLastDay=True)
        assert dc.dayCount(d1, d2) == 183
        assert dc.yearFraction(d1, d2) == pytest.approx(183.0 / 360.0)


class TestActual364:
    def test_name(self):
        assert Actual364().name() == "Actual/364"

    def test_year_fraction(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        assert Actual364().yearFraction(d1, d2) == pytest.approx(182.0 / 364.0)


class TestActual365Fixed:
    def test_name(self):
        assert Actual365Fixed().name() == "Actual/365 (Fixed)"
        assert Actual365Fixed(Actual365Fixed.Convention.NoLeap).name() == "Actual/365 (No Leap)"
        assert Actual365Fixed(Actual365Fixed.Convention.Canadian).name() == "Actual/365 (Fixed) Canadian Bond"

    def test_standard_year_fraction(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        assert Actual365Fixed().yearFraction(d1, d2) == pytest.approx(182.0 / 365.0)

    def test_no_leap_day_count(self):
        dc = Actual365Fixed(Actual365Fixed.Convention.NoLeap)
        # Feb 28 to Mar 1 in leap year: should count 1 day (skipping Feb 29)
        d1 = Date(28, Month.February, 2020)
        d2 = Date(1, Month.March, 2020)
        assert dc.dayCount(d1, d2) == 1

    def test_no_leap_year_fraction(self):
        dc = Actual365Fixed(Actual365Fixed.Convention.NoLeap)
        d1 = Date(1, Month.January, 2020)
        d2 = Date(31, Month.December, 2020)
        # 365 actual days but NoLeap removes Feb 29
        assert dc.yearFraction(d1, d2) == pytest.approx(dc.dayCount(d1, d2) / 365.0)


class TestActual36525:
    def test_name(self):
        assert Actual36525().name() == "Actual/365.25"

    def test_year_fraction(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        assert Actual36525().yearFraction(d1, d2) == pytest.approx(182.0 / 365.25)


class TestActual366:
    def test_name(self):
        assert Actual366().name() == "Actual/366"

    def test_year_fraction(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        assert Actual366().yearFraction(d1, d2) == pytest.approx(182.0 / 366.0)


class TestOneDayCounter:
    def test_name(self):
        assert OneDayCounter().name() == "1/1"

    def test_same_date(self):
        d = Date(1, Month.January, 2020)
        dc = OneDayCounter()
        assert dc.dayCount(d, d) == 0
        assert dc.yearFraction(d, d) == 0.0

    def test_different_dates(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        dc = OneDayCounter()
        assert dc.dayCount(d1, d2) == 1
        assert dc.yearFraction(d1, d2) == 1.0


class TestSimpleDayCounter:
    def test_name(self):
        assert SimpleDayCounter().name() == "Simple"

    def test_same_year(self):
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        dc = SimpleDayCounter()
        # 2020 is leap so 366 days in year
        assert dc.yearFraction(d1, d2) == pytest.approx(182.0 / 366.0)

    def test_cross_year(self):
        d1 = Date(1, Month.November, 2019)
        d2 = Date(1, Month.February, 2020)
        dc = SimpleDayCounter()
        # 61 days from Nov 1 to Dec 31 = 61 days in 2019, 31 days in Jan 2020
        yf = dc.yearFraction(d1, d2)
        assert yf > 0.0


class TestActualActual:
    """Transpiled from upstream/test-suite/daycounters.cpp testActualActual."""

    TEST_CASES = [
        # (convention, d1, d2, refStart, refEnd, expected)
        # first example
        ("ISDA", (1, 11, 2003), (1, 5, 2004), None, None, 0.497724380567),
        ("ISMA", (1, 11, 2003), (1, 5, 2004), (1, 11, 2003), (1, 5, 2004), 0.500000000000),
        ("AFB", (1, 11, 2003), (1, 5, 2004), None, None, 0.497267759563),
        # short first calculation period (first period)
        ("ISDA", (1, 2, 1999), (1, 7, 1999), None, None, 0.410958904110),
        ("ISMA", (1, 2, 1999), (1, 7, 1999), (1, 7, 1998), (1, 7, 1999), 0.410958904110),
        ("AFB", (1, 2, 1999), (1, 7, 1999), None, None, 0.410958904110),
        # short first calculation period (second period)
        ("ISDA", (1, 7, 1999), (1, 7, 2000), None, None, 1.001377348600),
        ("ISMA", (1, 7, 1999), (1, 7, 2000), (1, 7, 1999), (1, 7, 2000), 1.000000000000),
        ("AFB", (1, 7, 1999), (1, 7, 2000), None, None, 1.000000000000),
        # long first calculation period (first period)
        ("ISDA", (15, 8, 2002), (15, 7, 2003), None, None, 0.915068493151),
        ("ISMA", (15, 8, 2002), (15, 7, 2003), (15, 1, 2003), (15, 7, 2003), 0.915760869565),
        ("AFB", (15, 8, 2002), (15, 7, 2003), None, None, 0.915068493151),
        # long first calculation period (second period)
        ("ISDA", (15, 7, 2003), (15, 1, 2004), None, None, 0.504004790778),
        ("ISMA", (15, 7, 2003), (15, 1, 2004), (15, 7, 2003), (15, 1, 2004), 0.500000000000),
        ("AFB", (15, 7, 2003), (15, 1, 2004), None, None, 0.504109589041),
        # short final calculation period (penultimate period)
        ("ISDA", (30, 7, 1999), (30, 1, 2000), None, None, 0.503892506924),
        ("ISMA", (30, 7, 1999), (30, 1, 2000), (30, 7, 1999), (30, 1, 2000), 0.500000000000),
        ("AFB", (30, 7, 1999), (30, 1, 2000), None, None, 0.504109589041),
        # short final calculation period (final period)
        ("ISDA", (30, 1, 2000), (30, 6, 2000), None, None, 0.415300546448),
        ("ISMA", (30, 1, 2000), (30, 6, 2000), (30, 1, 2000), (30, 7, 2000), 0.417582417582),
        ("AFB", (30, 1, 2000), (30, 6, 2000), None, None, 0.41530054644),
    ]

    @pytest.mark.parametrize("conv,d1,d2,ref1,ref2,expected", TEST_CASES)
    def test_actual_actual(self, conv, d1, d2, ref1, ref2, expected):
        convention_map = {
            "ISDA": ActualActual.Convention.ISDA,
            "ISMA": ActualActual.Convention.ISMA,
            "AFB": ActualActual.Convention.AFB,
        }
        dc = ActualActual(convention_map[conv])
        start = Date(d1[0], Month(d1[1]), d1[2])
        end = Date(d2[0], Month(d2[1]), d2[2])
        ref_start = Date(ref1[0], Month(ref1[1]), ref1[2]) if ref1 else None
        ref_end = Date(ref2[0], Month(ref2[1]), ref2[2]) if ref2 else None
        result = dc.yearFraction(start, end, ref_start, ref_end)
        assert result == pytest.approx(expected, abs=1e-10), (
            f"ActualActual({conv}): yearFraction({start}, {end}) = {result}, expected {expected}"
        )


class TestThirty360:
    def test_us_name(self):
        assert Thirty360(Thirty360.Convention.USA).name() == "30/360 (US)"

    def test_bond_basis_name(self):
        assert Thirty360(Thirty360.Convention.BondBasis).name() == "30/360 (Bond Basis)"

    def test_european_name(self):
        assert Thirty360(Thirty360.Convention.European).name() == "30E/360 (Eurobond Basis)"

    def test_us_basic(self):
        dc = Thirty360(Thirty360.Convention.USA)
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        # 30*6 = 180
        assert dc.dayCount(d1, d2) == 180
        assert dc.yearFraction(d1, d2) == pytest.approx(180.0 / 360.0)

    def test_us_end_of_february(self):
        dc = Thirty360(Thirty360.Convention.USA)
        # Feb 28 2019 to Mar 1 2019 (non-leap): last of Feb → dd1=30
        d1 = Date(28, Month.February, 2019)
        d2 = Date(1, Month.March, 2019)
        assert dc.dayCount(d1, d2) == 1

    def test_european_31st_rule(self):
        dc = Thirty360(Thirty360.Convention.European)
        d1 = Date(31, Month.January, 2020)
        d2 = Date(31, Month.March, 2020)
        # dd1=30, dd2=30 → 30*2 = 60
        assert dc.dayCount(d1, d2) == 60

    def test_isda_termination(self):
        term = Date(28, Month.February, 2020)
        dc = Thirty360(Thirty360.Convention.ISDA, terminationDate=term)
        assert dc.name() == "30E/360 (ISDA)"


class TestThirty365:
    def test_name(self):
        assert Thirty365().name() == "30/365"

    def test_day_count(self):
        dc = Thirty365()
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        assert dc.dayCount(d1, d2) == 180

    def test_year_fraction(self):
        dc = Thirty365()
        d1 = Date(1, Month.January, 2020)
        d2 = Date(1, Month.July, 2020)
        assert dc.yearFraction(d1, d2) == pytest.approx(180.0 / 365.0)


class TestBusiness252:
    def test_name(self):
        assert Business252().name() == "Business/252"

    def test_weekday_only(self):
        dc = Business252()
        # Jan 6 2020 (Monday) to Jan 10 2020 (Friday) = 4 business days
        d1 = Date(6, Month.January, 2020)
        d2 = Date(10, Month.January, 2020)
        assert dc.dayCount(d1, d2) == 4

    def test_over_weekend(self):
        dc = Business252()
        # Jan 10 2020 (Friday) to Jan 13 2020 (Monday) = 1 business day
        d1 = Date(10, Month.January, 2020)
        d2 = Date(13, Month.January, 2020)
        assert dc.dayCount(d1, d2) == 1

    def test_year_fraction(self):
        dc = Business252()
        d1 = Date(6, Month.January, 2020)
        d2 = Date(10, Month.January, 2020)
        assert dc.yearFraction(d1, d2) == pytest.approx(4.0 / 252.0)
