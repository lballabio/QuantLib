"""SWIG-flavored compatibility helpers for qlnb.

This module is a **best-effort shim**, not full QuantLib-SWIG parity. Prefer the
native snake_case qlnb API for new code. Import style:

    import qlnb.compat as ql
    # or
    from qlnb import compat as ql

Useful aliases include module-level months (`ql.May`), `ql.Option.Put`,
camelCase method aliases (`setPricingEngine`, `cleanPrice`), and
`Settings.instance().evaluationDate`.
"""

from __future__ import annotations

from typing import Any

import qlnb as _ql
from qlnb import *  # noqa: F403

# Re-export version from the native package.
__version__ = _ql.__version__

# ---------------------------------------------------------------------------
# Module-level month aliases (SWIG: ql.May)
# ---------------------------------------------------------------------------
January = _ql.Month.January
February = _ql.Month.February
March = _ql.Month.March
April = _ql.Month.April
May = _ql.Month.May
June = _ql.Month.June
July = _ql.Month.July
August = _ql.Month.August
September = _ql.Month.September
October = _ql.Month.October
November = _ql.Month.November
December = _ql.Month.December


# ---------------------------------------------------------------------------
# Option.Put / Option.Call (SWIG nests under Option, qlnb uses OptionType)
# ---------------------------------------------------------------------------
class Option:
    """SWIG-style Option.Put / Option.Call namespace."""

    Put = _ql.OptionType.Put
    Call = _ql.OptionType.Call


# ---------------------------------------------------------------------------
# Settings.evaluationDate camelCase property
# ---------------------------------------------------------------------------
class _SettingsProxy:
    """Thin proxy adding SWIG-style ``evaluationDate`` to Settings."""

    __slots__ = ("_settings",)

    def __init__(self, settings: Any) -> None:
        self._settings = settings

    @property
    def evaluation_date(self) -> Any:
        return self._settings.evaluation_date

    @evaluation_date.setter
    def evaluation_date(self, date: Any) -> None:
        self._settings.evaluation_date = date

    @property
    def evaluationDate(self) -> Any:  # noqa: N802 — SWIG alias
        return self._settings.evaluation_date

    @evaluationDate.setter
    def evaluationDate(self, date: Any) -> None:  # noqa: N802 — SWIG alias
        self._settings.evaluation_date = date

    def anchor_evaluation_date(self) -> None:
        self._settings.anchor_evaluation_date()

    def reset_evaluation_date(self) -> None:
        self._settings.reset_evaluation_date()

    def anchorEvaluationDate(self) -> None:  # noqa: N802
        self._settings.anchor_evaluation_date()

    def resetEvaluationDate(self) -> None:  # noqa: N802
        self._settings.reset_evaluation_date()


class Settings:
    """SWIG-style Settings with ``evaluationDate`` alias."""

    @staticmethod
    def instance() -> _SettingsProxy:
        return _SettingsProxy(_ql.Settings.instance())


# Module-level Settings helpers matching SWIG naming.
def evaluationDate() -> Any:  # noqa: N802
    """Return the current evaluation date (SWIG-style helper name)."""
    return _ql.get_evaluation_date()


# ---------------------------------------------------------------------------
# CamelCase method aliases on concrete classes
# ---------------------------------------------------------------------------
def _install_aliases() -> None:
    Date = _ql.Date
    Date.dayOfMonth = Date.day_of_month  # type: ignore[attr-defined]
    Date.serialNumber = Date.serial_number  # type: ignore[attr-defined]
    Date.todaysDate = staticmethod(Date.todays_date)  # type: ignore[attr-defined]
    Date.minDate = staticmethod(Date.min_date)  # type: ignore[attr-defined]
    Date.maxDate = staticmethod(Date.max_date)  # type: ignore[attr-defined]

    SimpleQuote = _ql.SimpleQuote
    SimpleQuote.setValue = SimpleQuote.set_value  # type: ignore[attr-defined]
    SimpleQuote.isValid = SimpleQuote.is_valid  # type: ignore[attr-defined]

    Quote = _ql.Quote
    Quote.isValid = Quote.is_valid  # type: ignore[attr-defined]

    QuoteHandle = _ql.QuoteHandle
    QuoteHandle.currentLink = QuoteHandle.current_link  # type: ignore[attr-defined]

    RelinkableQuoteHandle = _ql.RelinkableQuoteHandle
    RelinkableQuoteHandle.currentLink = RelinkableQuoteHandle.current_link  # type: ignore[attr-defined]
    RelinkableQuoteHandle.linkTo = RelinkableQuoteHandle.link_to  # type: ignore[attr-defined]

    DayCounter = _ql.DayCounter
    DayCounter.yearFraction = DayCounter.year_fraction  # type: ignore[attr-defined]
    DayCounter.dayCount = DayCounter.day_count  # type: ignore[attr-defined]

    Calendar = _ql.Calendar
    Calendar.isBusinessDay = Calendar.is_business_day  # type: ignore[attr-defined]
    Calendar.isHoliday = Calendar.is_holiday  # type: ignore[attr-defined]

    Schedule = _ql.Schedule
    Schedule.startDate = Schedule.start_date  # type: ignore[attr-defined]
    Schedule.endDate = Schedule.end_date  # type: ignore[attr-defined]

    YieldTermStructureHandle = _ql.YieldTermStructureHandle
    YieldTermStructureHandle.referenceDate = (  # type: ignore[attr-defined]
        YieldTermStructureHandle.reference_date
    )
    YieldTermStructureHandle.zeroRate = YieldTermStructureHandle.zero_rate  # type: ignore[attr-defined]

    FixedRateBond = _ql.FixedRateBond
    FixedRateBond.cleanPrice = FixedRateBond.clean_price  # type: ignore[attr-defined]
    FixedRateBond.dirtyPrice = FixedRateBond.dirty_price  # type: ignore[attr-defined]
    FixedRateBond.settlementDate = FixedRateBond.settlement_date  # type: ignore[attr-defined]
    FixedRateBond.maturityDate = FixedRateBond.maturity_date  # type: ignore[attr-defined]
    FixedRateBond.setPricingEngine = FixedRateBond.set_pricing_engine  # type: ignore[attr-defined]

    ZeroCouponBond = getattr(_ql, "ZeroCouponBond", None)
    if ZeroCouponBond is not None:
        ZeroCouponBond.cleanPrice = ZeroCouponBond.clean_price  # type: ignore[attr-defined]
        ZeroCouponBond.dirtyPrice = ZeroCouponBond.dirty_price  # type: ignore[attr-defined]
        ZeroCouponBond.settlementDate = ZeroCouponBond.settlement_date  # type: ignore[attr-defined]
        ZeroCouponBond.maturityDate = ZeroCouponBond.maturity_date  # type: ignore[attr-defined]
        ZeroCouponBond.setPricingEngine = ZeroCouponBond.set_pricing_engine  # type: ignore[attr-defined]

    FloatingRateBond = getattr(_ql, "FloatingRateBond", None)
    if FloatingRateBond is not None:
        FloatingRateBond.cleanPrice = FloatingRateBond.clean_price  # type: ignore[attr-defined]
        FloatingRateBond.dirtyPrice = FloatingRateBond.dirty_price  # type: ignore[attr-defined]
        FloatingRateBond.settlementDate = FloatingRateBond.settlement_date  # type: ignore[attr-defined]
        FloatingRateBond.maturityDate = FloatingRateBond.maturity_date  # type: ignore[attr-defined]
        FloatingRateBond.setPricingEngine = (  # type: ignore[attr-defined]
            FloatingRateBond.set_pricing_engine
        )

    VanillaSwap = _ql.VanillaSwap
    VanillaSwap.fairRate = VanillaSwap.fair_rate  # type: ignore[attr-defined]
    VanillaSwap.fairSpread = VanillaSwap.fair_spread  # type: ignore[attr-defined]
    VanillaSwap.setPricingEngine = VanillaSwap.set_pricing_engine  # type: ignore[attr-defined]

    Swaption = getattr(_ql, "Swaption", None)
    if Swaption is not None:
        Swaption.setPricingEngine = Swaption.set_pricing_engine  # type: ignore[attr-defined]
        Swaption.settlementType = Swaption.settlement_type  # type: ignore[attr-defined]
        Swaption.settlementMethod = Swaption.settlement_method  # type: ignore[attr-defined]
        Swaption.isExpired = Swaption.is_expired  # type: ignore[attr-defined]

    EuropeanOption = _ql.EuropeanOption
    EuropeanOption.setPricingEngine = EuropeanOption.set_pricing_engine  # type: ignore[attr-defined]
    EuropeanOption.impliedVolatility = EuropeanOption.implied_volatility  # type: ignore[attr-defined]

    VanillaOption = _ql.VanillaOption
    VanillaOption.setPricingEngine = VanillaOption.set_pricing_engine  # type: ignore[attr-defined]
    if hasattr(VanillaOption, "set_binomial_pricing_engine"):
        VanillaOption.setBinomialPricingEngine = (  # type: ignore[attr-defined]
            VanillaOption.set_binomial_pricing_engine
        )
    if hasattr(VanillaOption, "set_fd_pricing_engine"):
        VanillaOption.setFdPricingEngine = (  # type: ignore[attr-defined]
            VanillaOption.set_fd_pricing_engine
        )

    OvernightIndexedSwap = getattr(_ql, "OvernightIndexedSwap", None)
    if OvernightIndexedSwap is not None:
        OvernightIndexedSwap.fairRate = OvernightIndexedSwap.fair_rate  # type: ignore[attr-defined]
        OvernightIndexedSwap.fairSpread = (  # type: ignore[attr-defined]
            OvernightIndexedSwap.fair_spread
        )
        OvernightIndexedSwap.setPricingEngine = (  # type: ignore[attr-defined]
            OvernightIndexedSwap.set_pricing_engine
        )

    IborIndex = _ql.IborIndex
    IborIndex.fixingCalendar = IborIndex.fixing_calendar  # type: ignore[attr-defined]
    IborIndex.dayCounter = IborIndex.day_counter  # type: ignore[attr-defined]
    IborIndex.fixingDays = IborIndex.fixing_days  # type: ignore[attr-defined]
    IborIndex.addFixing = IborIndex.add_fixing  # type: ignore[attr-defined]

    OvernightIndex = getattr(_ql, "OvernightIndex", None)
    if OvernightIndex is not None:
        OvernightIndex.fixingCalendar = OvernightIndex.fixing_calendar  # type: ignore[attr-defined]
        OvernightIndex.dayCounter = OvernightIndex.day_counter  # type: ignore[attr-defined]
        OvernightIndex.fixingDays = OvernightIndex.fixing_days  # type: ignore[attr-defined]
        OvernightIndex.addFixing = OvernightIndex.add_fixing  # type: ignore[attr-defined]

    PlainVanillaPayoff = _ql.PlainVanillaPayoff
    PlainVanillaPayoff.optionType = PlainVanillaPayoff.option_type  # type: ignore[attr-defined]

    EuropeanExercise = _ql.EuropeanExercise
    EuropeanExercise.lastDate = EuropeanExercise.last_date  # type: ignore[attr-defined]

    AmericanExercise = _ql.AmericanExercise
    AmericanExercise.lastDate = AmericanExercise.last_date  # type: ignore[attr-defined]

    ForwardRateAgreement = _ql.ForwardRateAgreement
    ForwardRateAgreement.forwardRate = ForwardRateAgreement.forward_rate  # type: ignore[attr-defined]
    ForwardRateAgreement.fixingDate = ForwardRateAgreement.fixing_date  # type: ignore[attr-defined]

    # Phase-4 experimental instruments (present when bindings are built).
    BarrierOption = getattr(_ql, "BarrierOption", None)
    if BarrierOption is not None:
        BarrierOption.setPricingEngine = BarrierOption.set_pricing_engine  # type: ignore[attr-defined]

    CapFloor = getattr(_ql, "CapFloor", None)
    if CapFloor is not None:
        CapFloor.setPricingEngine = CapFloor.set_pricing_engine  # type: ignore[attr-defined]
        CapFloor.atmRate = CapFloor.atm_rate  # type: ignore[attr-defined]
        CapFloor.startDate = CapFloor.start_date  # type: ignore[attr-defined]
        CapFloor.maturityDate = CapFloor.maturity_date  # type: ignore[attr-defined]


_install_aliases()


# Convenience aliases matching common SWIG free functions / names.
setEvaluationDate = _ql.set_evaluation_date  # noqa: N816
getEvaluationDate = _ql.get_evaluation_date  # noqa: N816

# Settlement nested namespace (SWIG: ql.Settlement.Physical).
class Settlement:
    """SWIG-style Settlement.Physical / Settlement.Cash namespace."""

    Physical = getattr(_ql, "SettlementType").Physical
    Cash = getattr(_ql, "SettlementType").Cash
    PhysicalOTC = getattr(_ql, "SettlementMethod").PhysicalOTC
    PhysicalCleared = getattr(_ql, "SettlementMethod").PhysicalCleared
    CollateralizedCashPrice = getattr(_ql, "SettlementMethod").CollateralizedCashPrice
    ParYieldCurve = getattr(_ql, "SettlementMethod").ParYieldCurve


# Attach Payer/Receiver on VanillaSwap class for SWIG-like access.
_ql.VanillaSwap.Payer = _ql.SwapType.Payer  # type: ignore[attr-defined]
_ql.VanillaSwap.Receiver = _ql.SwapType.Receiver  # type: ignore[attr-defined]

# Phase-6 MakeOIS-style alias.
makeOIS = getattr(_ql, "make_ois", None)


__all__ = [name for name in globals() if not name.startswith("_")]
