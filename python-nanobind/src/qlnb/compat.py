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
        if hasattr(Swaption, "set_tree_pricing_engine"):
            Swaption.setTreePricingEngine = (  # type: ignore[attr-defined]
                Swaption.set_tree_pricing_engine
            )
        if hasattr(Swaption, "set_jamshidian_pricing_engine"):
            Swaption.setJamshidianPricingEngine = (  # type: ignore[attr-defined]
                Swaption.set_jamshidian_pricing_engine
            )
        if hasattr(Swaption, "set_gaussian1d_pricing_engine"):
            Swaption.setGaussian1dPricingEngine = (  # type: ignore[attr-defined]
                Swaption.set_gaussian1d_pricing_engine
            )
        if hasattr(Swaption, "set_fd_hullwhite_pricing_engine"):
            Swaption.setFdHullWhitePricingEngine = (  # type: ignore[attr-defined]
                Swaption.set_fd_hullwhite_pricing_engine
            )

    CreditDefaultSwap = getattr(_ql, "CreditDefaultSwap", None)
    if CreditDefaultSwap is not None:
        CreditDefaultSwap.fairSpread = CreditDefaultSwap.fair_spread  # type: ignore[attr-defined]
        CreditDefaultSwap.fairUpfront = CreditDefaultSwap.fair_upfront  # type: ignore[attr-defined]
        CreditDefaultSwap.couponLegNPV = (  # type: ignore[attr-defined]
            CreditDefaultSwap.coupon_leg_NPV
        )
        CreditDefaultSwap.defaultLegNPV = (  # type: ignore[attr-defined]
            CreditDefaultSwap.default_leg_NPV
        )
        CreditDefaultSwap.runningSpread = (  # type: ignore[attr-defined]
            CreditDefaultSwap.running_spread
        )
        CreditDefaultSwap.setPricingEngine = (  # type: ignore[attr-defined]
            CreditDefaultSwap.set_pricing_engine
        )
        CreditDefaultSwap.isExpired = CreditDefaultSwap.is_expired  # type: ignore[attr-defined]
        if hasattr(CreditDefaultSwap, "set_isda_pricing_engine"):
            CreditDefaultSwap.setIsdaPricingEngine = (  # type: ignore[attr-defined]
                CreditDefaultSwap.set_isda_pricing_engine
            )

    BermudanExercise = getattr(_ql, "BermudanExercise", None)
    if BermudanExercise is not None:
        BermudanExercise.lastDate = BermudanExercise.last_date  # type: ignore[attr-defined]

    DefaultProbabilityTermStructureHandle = getattr(
        _ql, "DefaultProbabilityTermStructureHandle", None
    )
    if DefaultProbabilityTermStructureHandle is not None:
        DefaultProbabilityTermStructureHandle.survivalProbability = (  # type: ignore[attr-defined]
            DefaultProbabilityTermStructureHandle.survival_probability
        )
        DefaultProbabilityTermStructureHandle.hazardRate = (  # type: ignore[attr-defined]
            DefaultProbabilityTermStructureHandle.hazard_rate
        )
        DefaultProbabilityTermStructureHandle.referenceDate = (  # type: ignore[attr-defined]
            DefaultProbabilityTermStructureHandle.reference_date
        )
        if hasattr(DefaultProbabilityTermStructureHandle, "default_probability"):
            DefaultProbabilityTermStructureHandle.defaultProbability = (  # type: ignore[attr-defined]
                DefaultProbabilityTermStructureHandle.default_probability
            )
        if hasattr(DefaultProbabilityTermStructureHandle, "max_date"):
            DefaultProbabilityTermStructureHandle.maxDate = (  # type: ignore[attr-defined]
                DefaultProbabilityTermStructureHandle.max_date
            )

    Gsr = getattr(_ql, "Gsr", None)
    if Gsr is not None and hasattr(Gsr, "numeraire_time"):
        Gsr.numeraireTime = Gsr.numeraire_time  # type: ignore[attr-defined]

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

# Phase-7 Protection nested namespace (SWIG: ql.Protection.Seller).
class Protection:
    """SWIG-style Protection.Buyer / Protection.Seller namespace."""

    Buyer = getattr(_ql, "ProtectionSide").Buyer
    Seller = getattr(_ql, "ProtectionSide").Seller

# FD mesher / value-grid NumPy helpers (camelCase aliases).
uniform1dMesherLocations = getattr(_ql, "uniform_1d_mesher_locations", None)
fdmBlackScholesMesherLocations = getattr(
    _ql, "fdm_black_scholes_mesher_locations", None
)
fdmBlackScholesValues = getattr(_ql, "fdm_black_scholes_values", None)

# Phase-9 CDS bootstrap / Asian aliases.
SpreadCdsHelper = getattr(_ql, "SpreadCdsHelper", None)
PiecewiseHazardRateCurve = getattr(_ql, "PiecewiseHazardRateCurve", None)
ContinuousAveragingAsianOption = getattr(
    _ql, "ContinuousAveragingAsianOption", None
)
DiscreteAveragingAsianOption = getattr(
    _ql, "DiscreteAveragingAsianOption", None
)
if ContinuousAveragingAsianOption is not None:
    ContinuousAveragingAsianOption.setPricingEngine = (  # type: ignore[attr-defined]
        ContinuousAveragingAsianOption.set_pricing_engine
    )
if DiscreteAveragingAsianOption is not None:
    DiscreteAveragingAsianOption.setPricingEngine = (  # type: ignore[attr-defined]
        DiscreteAveragingAsianOption.set_pricing_engine
    )

# Phase-10 CMS / SwapIndex aliases.
EuriborSwapIsdaFixA = getattr(_ql, "EuriborSwapIsdaFixA", None)
ConstantSwaptionVolatility = getattr(_ql, "ConstantSwaptionVolatility", None)
AnalyticHaganPricer = getattr(_ql, "AnalyticHaganPricer", None)
NumericHaganPricer = getattr(_ql, "NumericHaganPricer", None)
makeCms = getattr(_ql, "make_cms", None)

CmsCoupon = getattr(_ql, "CmsCoupon", None)
if CmsCoupon is not None:
    CmsCoupon.setPricer = CmsCoupon.set_pricer  # type: ignore[attr-defined]
    CmsCoupon.accrualStartDate = CmsCoupon.accrual_start_date  # type: ignore[attr-defined]
    CmsCoupon.accrualEndDate = CmsCoupon.accrual_end_date  # type: ignore[attr-defined]

Swap = getattr(_ql, "Swap", None)
if Swap is not None:
    Swap.setPricingEngine = Swap.set_pricing_engine  # type: ignore[attr-defined]
    Swap.isExpired = Swap.is_expired  # type: ignore[attr-defined]
    Swap.numberOfLegs = Swap.number_of_legs  # type: ignore[attr-defined]
    if hasattr(Swap, "set_cms_coupon_pricer"):
        Swap.setCmsCouponPricer = Swap.set_cms_coupon_pricer  # type: ignore[attr-defined]

SwapIndex = getattr(_ql, "SwapIndex", None)
if SwapIndex is not None:
    SwapIndex.fixingDays = SwapIndex.fixing_days  # type: ignore[attr-defined]
    SwapIndex.fixingCalendar = SwapIndex.fixing_calendar  # type: ignore[attr-defined]
    SwapIndex.dayCounter = SwapIndex.day_counter  # type: ignore[attr-defined]
    if hasattr(SwapIndex, "add_fixing"):
        SwapIndex.addFixing = SwapIndex.add_fixing  # type: ignore[attr-defined]
    if hasattr(SwapIndex, "value_date"):
        SwapIndex.valueDate = SwapIndex.value_date  # type: ignore[attr-defined]

# Phase-11 CMS-spread aliases.
LinearTsrPricer = getattr(_ql, "LinearTsrPricer", None)
LognormalCmsSpreadPricer = getattr(_ql, "LognormalCmsSpreadPricer", None)
SwapSpreadIndex = getattr(_ql, "make_swap_spread_index", None)
CmsSpreadCoupon = getattr(_ql, "CmsSpreadCoupon", None)
if CmsSpreadCoupon is not None:
    CmsSpreadCoupon.setPricer = CmsSpreadCoupon.set_pricer  # type: ignore[attr-defined]
    CmsSpreadCoupon.fixingDate = CmsSpreadCoupon.fixing_date  # type: ignore[attr-defined]
CappedFlooredCmsSpreadCoupon = getattr(_ql, "CappedFlooredCmsSpreadCoupon", None)
if CappedFlooredCmsSpreadCoupon is not None:
    CappedFlooredCmsSpreadCoupon.setPricer = (  # type: ignore[attr-defined]
        CappedFlooredCmsSpreadCoupon.set_pricer
    )

# Phase-12 zero-inflation / ZCIS aliases.
UKRPI = getattr(_ql, "UKRPI", None)
EUHICP = getattr(_ql, "EUHICP", None)
ZeroCouponInflationSwapHelper = getattr(
    _ql, "ZeroCouponInflationSwapHelper", None
)
PiecewiseZeroInflationCurve = getattr(_ql, "PiecewiseZeroInflationCurve", None)
InterpolatedZeroInflationCurve = getattr(
    _ql, "InterpolatedZeroInflationCurve", None
)
FlatZeroInflationCurve = getattr(_ql, "FlatZeroInflationCurve", None)

ZeroInflationIndex = getattr(_ql, "ZeroInflationIndex", None)
if ZeroInflationIndex is not None:
    ZeroInflationIndex.addFixing = ZeroInflationIndex.add_fixing  # type: ignore[attr-defined]
    ZeroInflationIndex.lastFixingDate = (  # type: ignore[attr-defined]
        ZeroInflationIndex.last_fixing_date
    )
    ZeroInflationIndex.availabilityLag = (  # type: ignore[attr-defined]
        ZeroInflationIndex.availability_lag
    )

ZeroCouponInflationSwap = getattr(_ql, "ZeroCouponInflationSwap", None)
if ZeroCouponInflationSwap is not None:
    ZeroCouponInflationSwap.setPricingEngine = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.set_pricing_engine
    )
    ZeroCouponInflationSwap.fairRate = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.fair_rate
    )
    ZeroCouponInflationSwap.fixedRate = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.fixed_rate
    )
    ZeroCouponInflationSwap.fixedLegNPV = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.fixed_leg_NPV
    )
    ZeroCouponInflationSwap.inflationLegNPV = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.inflation_leg_NPV
    )
    ZeroCouponInflationSwap.startDate = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.start_date
    )
    ZeroCouponInflationSwap.maturityDate = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.maturity_date
    )
    ZeroCouponInflationSwap.isExpired = (  # type: ignore[attr-defined]
        ZeroCouponInflationSwap.is_expired
    )

ZeroInflationTermStructureHandle = getattr(
    _ql, "ZeroInflationTermStructureHandle", None
)
if ZeroInflationTermStructureHandle is not None:
    ZeroInflationTermStructureHandle.zeroRate = (  # type: ignore[attr-defined]
        ZeroInflationTermStructureHandle.zero_rate
    )
    ZeroInflationTermStructureHandle.baseDate = (  # type: ignore[attr-defined]
        ZeroInflationTermStructureHandle.base_date
    )
    ZeroInflationTermStructureHandle.maxDate = (  # type: ignore[attr-defined]
        ZeroInflationTermStructureHandle.max_date
    )
    ZeroInflationTermStructureHandle.referenceDate = (  # type: ignore[attr-defined]
        ZeroInflationTermStructureHandle.reference_date
    )

RelinkableZeroInflationTermStructureHandle = getattr(
    _ql, "RelinkableZeroInflationTermStructureHandle", None
)
if RelinkableZeroInflationTermStructureHandle is not None:
    RelinkableZeroInflationTermStructureHandle.linkTo = (  # type: ignore[attr-defined]
        RelinkableZeroInflationTermStructureHandle.link_to
    )
    RelinkableZeroInflationTermStructureHandle.asHandle = (  # type: ignore[attr-defined]
        RelinkableZeroInflationTermStructureHandle.as_handle
    )

# SWIG-style CPI.Flat nested namespace.
class CPI:
    """SWIG-style CPI.Flat / CPI.Linear namespace."""

    Flat = getattr(_ql, "CPIInterpolationType").Flat
    Linear = getattr(_ql, "CPIInterpolationType").Linear

# SWIG-style GFunctionFactory.Standard nested namespace.
class GFunctionFactory:
    """SWIG-style GFunctionFactory.Standard / ExactYield / … namespace."""

    Standard = getattr(_ql, "YieldCurveModel").Standard
    ExactYield = getattr(_ql, "YieldCurveModel").ExactYield
    ParallelShifts = getattr(_ql, "YieldCurveModel").ParallelShifts
    NonParallelShifts = getattr(_ql, "YieldCurveModel").NonParallelShifts


__all__ = [name for name in globals() if not name.startswith("_")]
