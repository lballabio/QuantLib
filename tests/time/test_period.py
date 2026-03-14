"""Tests for Period class.

Transpiled from upstream/test-suite/period.cpp
"""

import pytest

from quantlib.time.frequency import Frequency
from quantlib.time.period import Period, days, months, weeks, years
from quantlib.time.timeunit import TimeUnit


class TestYearsMonthsAlgebra:
    def test_division(self):
        one_year = Period(1, TimeUnit.Years)
        six_months = Period(6, TimeUnit.Months)
        three_months = Period(3, TimeUnit.Months)

        assert one_year / 4 == three_months
        assert one_year / 2 == six_months

    def test_addition(self):
        three_months = Period(3, TimeUnit.Months)
        six_months = Period(6, TimeUnit.Months)
        one_year = Period(1, TimeUnit.Years)

        result = three_months + six_months
        assert result == Period(9, TimeUnit.Months)

        result = result + one_year
        assert result == Period(21, TimeUnit.Months)

    def test_no_normalization_without_call(self):
        twelve_months = Period(12, TimeUnit.Months)
        assert twelve_months.length() == 12
        assert twelve_months.units() == TimeUnit.Months

    def test_normalize(self):
        twelve_months = Period(12, TimeUnit.Months)
        normalized = twelve_months.normalized()
        assert normalized.length() == 1
        assert normalized.units() == TimeUnit.Years


class TestWeeksDaysAlgebra:
    def test_division(self):
        two_weeks = Period(2, TimeUnit.Weeks)
        one_week = Period(1, TimeUnit.Weeks)
        one_day = Period(1, TimeUnit.Days)

        assert two_weeks / 2 == one_week
        assert one_week / 7 == one_day

    def test_addition(self):
        three_days = Period(3, TimeUnit.Days)
        one_day = Period(1, TimeUnit.Days)
        one_week = Period(1, TimeUnit.Weeks)

        result = three_days + one_day
        assert result == Period(4, TimeUnit.Days)

        result = result + one_week
        assert result == Period(11, TimeUnit.Days)

    def test_addition_with_zero(self):
        one_week = Period(1, TimeUnit.Weeks)
        zero_days = Period(0, TimeUnit.Days)
        assert one_week + zero_days == one_week

    def test_addition_weeks_and_days(self):
        one_week = Period(1, TimeUnit.Weeks)
        one_day = Period(1, TimeUnit.Days)
        two_weeks = Period(2, TimeUnit.Weeks)

        assert one_week + 3 * one_day == Period(10, TimeUnit.Days)
        assert one_week + 7 * one_day == two_weeks

    def test_no_normalization_without_call(self):
        seven_days = Period(7, TimeUnit.Days)
        assert seven_days.length() == 7
        assert seven_days.units() == TimeUnit.Days


class TestOperators:
    def test_multiply_assign(self):
        p = Period(3, TimeUnit.Days)
        p *= 2
        assert p == Period(6, TimeUnit.Days)

    def test_subtract_assign(self):
        p = Period(6, TimeUnit.Days)
        p -= Period(2, TimeUnit.Days)
        assert p == Period(4, TimeUnit.Days)


class TestConvertToYears:
    def test_years_from_years(self):
        assert years(Period(0, TimeUnit.Years)) == 0
        assert years(Period(1, TimeUnit.Years)) == 1
        assert years(Period(5, TimeUnit.Years)) == 5

    def test_years_from_months(self):
        assert years(Period(1, TimeUnit.Months)) == pytest.approx(1.0 / 12.0, abs=1e-15)
        assert years(Period(8, TimeUnit.Months)) == pytest.approx(8.0 / 12.0, abs=1e-15)
        assert years(Period(12, TimeUnit.Months)) == 1
        assert years(Period(18, TimeUnit.Months)) == pytest.approx(1.5, abs=1e-15)


class TestConvertToMonths:
    def test_months_from_months(self):
        assert months(Period(0, TimeUnit.Months)) == 0
        assert months(Period(1, TimeUnit.Months)) == 1
        assert months(Period(5, TimeUnit.Months)) == 5

    def test_months_from_years(self):
        assert months(Period(1, TimeUnit.Years)) == 12
        assert months(Period(3, TimeUnit.Years)) == 36


class TestConvertToDays:
    def test_days_from_days(self):
        assert days(Period(0, TimeUnit.Days)) == 0
        assert days(Period(1, TimeUnit.Days)) == 1
        assert days(Period(5, TimeUnit.Days)) == 5

    def test_days_from_weeks(self):
        assert days(Period(1, TimeUnit.Weeks)) == 7
        assert days(Period(3, TimeUnit.Weeks)) == 21


class TestConvertToWeeks:
    def test_weeks_from_weeks(self):
        assert weeks(Period(0, TimeUnit.Weeks)) == 0
        assert weeks(Period(1, TimeUnit.Weeks)) == 1
        assert weeks(Period(5, TimeUnit.Weeks)) == 5

    def test_weeks_from_days(self):
        assert weeks(Period(1, TimeUnit.Days)) == pytest.approx(1.0 / 7.0, abs=1e-15)
        assert weeks(Period(3, TimeUnit.Days)) == pytest.approx(3.0 / 7.0, abs=1e-15)
        assert weeks(Period(11, TimeUnit.Days)) == pytest.approx(11.0 / 7.0, abs=1e-15)


class TestNormalization:
    def test_normalized_equals_original(self):
        test_values = [
            Period(0, TimeUnit.Days),
            Period(0, TimeUnit.Weeks),
            Period(0, TimeUnit.Months),
            Period(0, TimeUnit.Years),
            Period(3, TimeUnit.Days),
            Period(7, TimeUnit.Days),
            Period(14, TimeUnit.Days),
            Period(30, TimeUnit.Days),
            Period(60, TimeUnit.Days),
            Period(365, TimeUnit.Days),
            Period(1, TimeUnit.Weeks),
            Period(2, TimeUnit.Weeks),
            Period(4, TimeUnit.Weeks),
            Period(8, TimeUnit.Weeks),
            Period(52, TimeUnit.Weeks),
            Period(1, TimeUnit.Months),
            Period(2, TimeUnit.Months),
            Period(6, TimeUnit.Months),
            Period(12, TimeUnit.Months),
            Period(18, TimeUnit.Months),
            Period(24, TimeUnit.Months),
            Period(1, TimeUnit.Years),
            Period(2, TimeUnit.Years),
        ]

        for p1 in test_values:
            n1 = p1.normalized()
            assert n1 == p1, f"Normalizing {p1} yields {n1}, which compares different"

    def test_equal_periods_normalize_same(self):
        """Periods which compare equal must normalize to the same period."""
        test_values = [
            Period(0, TimeUnit.Days),
            Period(0, TimeUnit.Weeks),
            Period(0, TimeUnit.Months),
            Period(0, TimeUnit.Years),
            Period(7, TimeUnit.Days),
            Period(14, TimeUnit.Days),
            Period(1, TimeUnit.Weeks),
            Period(2, TimeUnit.Weeks),
            Period(12, TimeUnit.Months),
            Period(24, TimeUnit.Months),
            Period(1, TimeUnit.Years),
            Period(2, TimeUnit.Years),
        ]

        for p1 in test_values:
            n1 = p1.normalized()
            for p2 in test_values:
                n2 = p2.normalized()
                try:
                    equal = p1 == p2
                except Exception:
                    continue
                if equal:
                    assert n1.units() == n2.units() and n1.length() == n2.length(), (
                        f"{p1} and {p2} compare equal, but normalize to {n1} and {n2}"
                    )


class TestFrequencyComputation:
    def test_roundtrip(self):
        """frequency -> period -> frequency == initial frequency"""
        for f in [
            Frequency.NoFrequency,
            Frequency.Once,
            Frequency.Annual,
            Frequency.Semiannual,
            Frequency.EveryFourthMonth,
            Frequency.Quarterly,
            Frequency.Bimonthly,
            Frequency.Monthly,
            Frequency.EveryFourthWeek,
            Frequency.Biweekly,
            Frequency.Weekly,
            Frequency.Daily,
        ]:
            assert Period(f).frequency() == f, f"Roundtrip failed for {f}"

    def test_other_frequency_raises(self):
        with pytest.raises(ValueError):
            Period(Frequency.OtherFrequency)

    def test_period_to_frequency(self):
        assert Period(1, TimeUnit.Years).frequency() == Frequency.Annual
        assert Period(6, TimeUnit.Months).frequency() == Frequency.Semiannual
        assert Period(4, TimeUnit.Months).frequency() == Frequency.EveryFourthMonth
        assert Period(3, TimeUnit.Months).frequency() == Frequency.Quarterly
        assert Period(2, TimeUnit.Months).frequency() == Frequency.Bimonthly
        assert Period(1, TimeUnit.Months).frequency() == Frequency.Monthly
        assert Period(4, TimeUnit.Weeks).frequency() == Frequency.EveryFourthWeek
        assert Period(2, TimeUnit.Weeks).frequency() == Frequency.Biweekly
        assert Period(1, TimeUnit.Weeks).frequency() == Frequency.Weekly
        assert Period(1, TimeUnit.Days).frequency() == Frequency.Daily


class TestStringRepresentation:
    def test_short_format(self):
        assert str(Period(1, TimeUnit.Days)) == "1D"
        assert str(Period(2, TimeUnit.Weeks)) == "2W"
        assert str(Period(3, TimeUnit.Months)) == "3M"
        assert str(Period(1, TimeUnit.Years)) == "1Y"
