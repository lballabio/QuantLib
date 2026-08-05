"""Phase-17 tests: inflation seasonality."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase17():
    assert ql.__version__ == "0.18.0"


_RPI_FIXINGS = [
    189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
    192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
    194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
    198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
    202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
    207.3,
]

_NON_UNIT_FACTORS = [
    1.003245,
    1.000000,
    0.999715,
    1.000495,
    1.000929,
    0.998687,
    0.995949,
    0.994682,
    0.995949,
    1.000519,
    1.003705,
    1.004186,
]


def _seasonality_market():
    # Mirrors test-suite/inflation.cpp::testSeasonalityCorrection.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)

    schedule = ql.Schedule(
        ql.Date(1, ql.Month.January, 2005),
        ql.Date(1, ql.Month.July, 2007),
        ql.Period(ql.Frequency.Monthly),
        ql.NullCalendar(),
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    for d, fixing in zip(list(schedule.dates()), _RPI_FIXINGS):
        index.add_fixing(d, fixing, force_overwrite=True)

    nodes = [
        ql.Date(1, ql.Month.June, 2007),
        ql.Date(1, ql.Month.June, 2008),
        ql.Date(1, ql.Month.June, 2009),
        ql.Date(1, ql.Month.June, 2010),
        ql.Date(1, ql.Month.June, 2011),
        ql.Date(1, ql.Month.June, 2012),
        ql.Date(1, ql.Month.June, 2014),
        ql.Date(1, ql.Month.June, 2017),
        ql.Date(1, ql.Month.June, 2019),
        ql.Date(1, ql.Month.June, 2022),
        ql.Date(1, ql.Month.June, 2027),
        ql.Date(1, ql.Month.June, 2032),
        ql.Date(1, ql.Month.June, 2037),
        ql.Date(1, ql.Month.June, 2047),
        ql.Date(1, ql.Month.June, 2057),
    ]
    rates = [
        0.0293, 0.0293, 0.0295, 0.02965, 0.0298, 0.03, 0.0306, 0.03175,
        0.03243, 0.03293, 0.03338, 0.03348, 0.03348, 0.03308, 0.03228,
    ]
    curve = ql.InterpolatedZeroInflationCurve(
        evaluation,
        nodes,
        rates,
        ql.Frequency.Monthly,
        ql.Thirty360(ql.Thirty360Convention.BondBasis),
    )
    hz.link_to(curve)
    return hz, index, curve


def test_inflation_period():
    start, end = ql.inflation_period(
        ql.Date(15, ql.Month.March, 2007), ql.Frequency.Monthly
    )
    assert start == ql.Date(1, ql.Month.March, 2007)
    assert end == ql.Date(31, ql.Month.March, 2007)


def test_multiplicative_seasonality_unit_and_non_unit():
    hz, index, curve = _seasonality_market()
    assert not curve.has_seasonality()

    _period_start, true_base = ql.inflation_period(
        curve.base_date(), ql.Frequency.Monthly
    )
    seasonality_base = ql.Date(31, ql.Month.January, true_base.year())

    unit = ql.MultiplicativePriceSeasonality(
        seasonality_base, ql.Frequency.Monthly, [1.0] * 12
    )
    non_unit = ql.MultiplicativePriceSeasonality(
        seasonality_base, ql.Frequency.Monthly, _NON_UNIT_FACTORS
    )

    fixing_dates = [
        ql.Date(14, ql.Month.January, 2013) + ql.Period(i, ql.TimeUnit.Months)
        for i in range(12)
    ]
    no_seasonality = [index.fixing(d, True) for d in fixing_dates]

    hz.set_seasonality(unit)
    assert hz.has_seasonality()
    unit_fixings = [index.fixing(d, True) for d in fixing_dates]
    for a, b in zip(no_seasonality, unit_fixings):
        assert a == pytest.approx(b, abs=1.0e-12)

    base_idx = curve.base_date().month().value - 1
    base_factor = _NON_UNIT_FACTORS[base_idx]
    # Still under unit seasonality (= no seasonality), compute expected.
    expected = [
        index.fixing(d, True) * _NON_UNIT_FACTORS[i] / base_factor
        for i, d in enumerate(fixing_dates)
    ]

    hz.set_seasonality(non_unit)
    actual = [index.fixing(d, True) for d in fixing_dates]
    for e, a in zip(expected, actual):
        assert a == pytest.approx(e, abs=1.0e-12)

    hz.set_seasonality()
    assert not hz.has_seasonality()
    cleared = [index.fixing(d, True) for d in fixing_dates]
    for a, b in zip(no_seasonality, cleared):
        assert a == pytest.approx(b, abs=1.0e-12)


def test_kerkhof_seasonality_factory():
    s = ql.KerkhofSeasonality(
        ql.Date(31, ql.Month.January, 2007), _NON_UNIT_FACTORS
    )
    assert s.frequency() == ql.Frequency.Monthly
    assert len(s.seasonality_factors()) == 12


def test_compat_phase17_aliases():
    import qlnb.compat as cql

    assert cql.MultiplicativePriceSeasonality is not None
    assert cql.KerkhofSeasonality is not None
    assert callable(cql.inflationPeriod)
    assert hasattr(cql.ZeroInflationTermStructureHandle, "setSeasonality")
    assert hasattr(cql.ZeroInflationTermStructureHandle, "hasSeasonality")
    assert hasattr(cql.MultiplicativePriceSeasonality, "seasonalityBaseDate")
