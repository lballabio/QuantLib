from typing import Iterable, Sequence, overload

class Month:
    January: Month
    February: Month
    March: Month
    April: Month
    May: Month
    June: Month
    July: Month
    August: Month
    September: Month
    October: Month
    November: Month
    December: Month

class TimeUnit:
    Days: TimeUnit
    Weeks: TimeUnit
    Months: TimeUnit
    Years: TimeUnit

class Frequency:
    Annual: Frequency
    Semiannual: Frequency
    Quarterly: Frequency
    Monthly: Frequency

class BusinessDayConvention:
    Following: BusinessDayConvention
    ModifiedFollowing: BusinessDayConvention
    Preceding: BusinessDayConvention
    ModifiedPreceding: BusinessDayConvention
    Unadjusted: BusinessDayConvention

class DateGeneration:
    Backward: DateGeneration
    Forward: DateGeneration

class Compounding:
    Simple: Compounding
    Compounded: Compounding
    Continuous: Compounding

class OptionType:
    Put: OptionType
    Call: OptionType

class SwapType:
    Receiver: SwapType
    Payer: SwapType

class Position:
    Long: Position
    Short: Position

class BarrierType:
    DownIn: BarrierType
    UpIn: BarrierType
    DownOut: BarrierType
    UpOut: BarrierType

class CapFloorType:
    Cap: CapFloorType
    Floor: CapFloorType
    Collar: CapFloorType

class ActualActualConvention:
    ISDA: ActualActualConvention
    ISMA: ActualActualConvention
    Bond: ActualActualConvention

class Thirty360Convention:
    BondBasis: Thirty360Convention
    USA: Thirty360Convention
    European: Thirty360Convention

class UnitedStatesMarket:
    Settlement: UnitedStatesMarket
    NYSE: UnitedStatesMarket
    GovernmentBond: UnitedStatesMarket
    SOFR: UnitedStatesMarket

class GermanyMarket:
    Settlement: GermanyMarket
    FrankfurtStockExchange: GermanyMarket
    Xetra: GermanyMarket
    Eurex: GermanyMarket
    Euwax: GermanyMarket

class SettlementType:
    Physical: SettlementType
    Cash: SettlementType

class SettlementMethod:
    PhysicalOTC: SettlementMethod
    PhysicalCleared: SettlementMethod
    CollateralizedCashPrice: SettlementMethod
    ParYieldCurve: SettlementMethod

class Date:
    def __init__(self, day: int, month: Month, year: int) -> None: ...
    def day_of_month(self) -> int: ...
    def month(self) -> Month: ...
    def year(self) -> int: ...
    def serial_number(self) -> int: ...
    def __add__(self, other: int | Period) -> Date: ...
    def __sub__(self, other: int | Period) -> Date: ...

class Period:
    def __init__(self, n: int, units: TimeUnit) -> None: ...
    def __init__(self, frequency: Frequency) -> None: ...  # type: ignore[misc]
    def length(self) -> int: ...
    def units(self) -> TimeUnit: ...
    def frequency(self) -> Frequency: ...
    def __mul__(self, n: int) -> Period: ...

class Settings:
    @staticmethod
    def instance() -> Settings: ...
    evaluation_date: Date

class Quote:
    def value(self) -> float: ...
    def is_valid(self) -> bool: ...

class SimpleQuote(Quote):
    def __init__(self, value: float) -> None: ...
    def set_value(self, value: float) -> float: ...

class QuoteHandle:
    def __init__(self, value: Quote = ...) -> None: ...
    def empty(self) -> bool: ...
    def current_link(self) -> Quote: ...

class DayCounter:
    def name(self) -> str: ...
    def year_fraction(self, d1: Date, d2: Date) -> float: ...
    def day_count(self, d1: Date, d2: Date) -> int: ...

class Calendar:
    def name(self) -> str: ...
    def is_business_day(self, date: Date) -> bool: ...
    def is_holiday(self, date: Date) -> bool: ...
    def advance(
        self,
        date: Date,
        period: Period | int,
        convention: BusinessDayConvention = ...,
        end_of_month: bool = ...,
        unit: TimeUnit = ...,
    ) -> Date: ...
    def adjust(self, date: Date, convention: BusinessDayConvention = ...) -> Date: ...

class Schedule:
    def __init__(
        self,
        effective_date: Date,
        termination_date: Date,
        tenor: Period,
        calendar: Calendar,
        convention: BusinessDayConvention,
        termination_date_convention: BusinessDayConvention,
        rule: DateGeneration,
        end_of_month: bool,
    ) -> None: ...
    def size(self) -> int: ...
    def __len__(self) -> int: ...
    def __getitem__(self, i: int) -> Date: ...
    def dates(self) -> list[Date]: ...
    def start_date(self) -> Date: ...
    def end_date(self) -> Date: ...

class InterestRate:
    def rate(self) -> float: ...
    def __float__(self) -> float: ...

class YieldTermStructureHandle:
    def empty(self) -> bool: ...
    def discount(self, date: Date, extrapolate: bool = ...) -> float: ...
    def reference_date(self) -> Date: ...
    def zero_rate(
        self,
        date: Date,
        day_counter: DayCounter,
        compounding: Compounding,
        frequency: Frequency = ...,
        extrapolate: bool = ...,
    ) -> InterestRate: ...

class RateHelper: ...

class IborIndex:
    def name(self) -> str: ...
    def tenor(self) -> Period: ...
    def fixing_calendar(self) -> Calendar: ...
    def day_counter(self) -> DayCounter: ...
    def fixing_days(self) -> int: ...
    def add_fixing(
        self, fixing_date: Date, fixing: float, force_overwrite: bool = ...
    ) -> None: ...
    def fixing(
        self, fixing_date: Date, forecast_todays_fixing: bool = ...
    ) -> float: ...

class OvernightIndex:
    def name(self) -> str: ...
    def tenor(self) -> Period: ...
    def fixing_calendar(self) -> Calendar: ...
    def day_counter(self) -> DayCounter: ...
    def fixing_days(self) -> int: ...
    def add_fixing(
        self, fixing_date: Date, fixing: float, force_overwrite: bool = ...
    ) -> None: ...
    def fixing(
        self, fixing_date: Date, forecast_todays_fixing: bool = ...
    ) -> float: ...

class FixedRateBond:
    def __init__(
        self,
        settlement_days: int,
        face_amount: float,
        schedule: Schedule,
        coupons: Sequence[float],
        accrual_day_counter: DayCounter,
        payment_convention: BusinessDayConvention = ...,
        redemption: float = ...,
        issue_date: Date = ...,
    ) -> None: ...
    def NPV(self) -> float: ...
    def clean_price(self) -> float: ...
    def dirty_price(self) -> float: ...
    def set_pricing_engine(self, discount_curve: YieldTermStructureHandle) -> None: ...

class ZeroCouponBond:
    def __init__(
        self,
        settlement_days: int,
        calendar: Calendar,
        face_amount: float,
        maturity_date: Date,
        payment_convention: BusinessDayConvention = ...,
        redemption: float = ...,
        issue_date: Date = ...,
    ) -> None: ...
    def NPV(self) -> float: ...
    def clean_price(self) -> float: ...
    def dirty_price(self) -> float: ...
    def settlement_date(self) -> Date: ...
    def maturity_date(self) -> Date: ...
    def set_pricing_engine(self, discount_curve: YieldTermStructureHandle) -> None: ...

class FloatingRateBond:
    def __init__(
        self,
        settlement_days: int,
        face_amount: float,
        schedule: Schedule,
        ibor_index: IborIndex,
        accrual_day_counter: DayCounter,
        payment_convention: BusinessDayConvention = ...,
        fixing_days: int = ...,
        gearings: Sequence[float] = ...,
        spreads: Sequence[float] = ...,
        caps: Sequence[float] = ...,
        floors: Sequence[float] = ...,
        in_arrears: bool = ...,
        redemption: float = ...,
        issue_date: Date = ...,
    ) -> None: ...
    def NPV(self) -> float: ...
    def clean_price(self) -> float: ...
    def dirty_price(self) -> float: ...
    def settlement_date(self) -> Date: ...
    def maturity_date(self) -> Date: ...
    def set_pricing_engine(self, discount_curve: YieldTermStructureHandle) -> None: ...

class VanillaSwap:
    def __init__(
        self,
        type: SwapType,
        nominal: float,
        fixed_schedule: Schedule,
        fixed_rate: float,
        fixed_day_count: DayCounter,
        float_schedule: Schedule,
        ibor_index: IborIndex,
        spread: float,
        floating_day_count: DayCounter,
    ) -> None: ...
    def NPV(self) -> float: ...
    def fair_rate(self) -> float: ...
    def fair_spread(self) -> float: ...
    def set_pricing_engine(self, discount_curve: YieldTermStructureHandle) -> None: ...

class EuropeanExercise:
    def __init__(self, date: Date) -> None: ...
    def last_date(self) -> Date: ...

class AmericanExercise:
    def __init__(
        self,
        earliest_date: Date,
        latest_date: Date,
        payoff_at_expiry: bool = ...,
    ) -> None: ...
    def __init__(self, latest_date: Date, payoff_at_expiry: bool = ...) -> None: ...  # type: ignore[misc]
    def last_date(self) -> Date: ...

class PlainVanillaPayoff:
    def __init__(self, type: OptionType, strike: float) -> None: ...
    def strike(self) -> float: ...
    def option_type(self) -> OptionType: ...

class BlackScholesMertonProcess:
    def __init__(
        self,
        x0: QuoteHandle,
        dividend_ts: YieldTermStructureHandle,
        risk_free_ts: YieldTermStructureHandle,
        black_vol_ts: object,
    ) -> None: ...

class EuropeanOption:
    def __init__(
        self, payoff: PlainVanillaPayoff, exercise: EuropeanExercise
    ) -> None: ...
    def NPV(self) -> float: ...
    def delta(self) -> float: ...
    def gamma(self) -> float: ...
    def vega(self) -> float: ...
    def implied_volatility(
        self,
        target_price: float,
        process: BlackScholesMertonProcess,
        accuracy: float = ...,
        max_evaluations: int = ...,
        min_vol: float = ...,
        max_vol: float = ...,
    ) -> float: ...
    def set_pricing_engine(self, process: BlackScholesMertonProcess) -> None: ...
    def set_mc_pricing_engine(
        self,
        process: BlackScholesMertonProcess,
        time_steps: int,
        required_samples: int,
        seed: int = ...,
        antithetic: bool = ...,
        brownian_bridge: bool = ...,
    ) -> None: ...

class VanillaOption:
    def __init__(
        self, payoff: PlainVanillaPayoff, exercise: AmericanExercise
    ) -> None: ...
    def __init__(
        self, payoff: PlainVanillaPayoff, exercise: EuropeanExercise
    ) -> None: ...  # type: ignore[misc]
    def NPV(self) -> float: ...
    def delta(self) -> float: ...
    def gamma(self) -> float: ...
    def vega(self) -> float: ...
    def set_pricing_engine(self, process: BlackScholesMertonProcess) -> None: ...
    def set_binomial_pricing_engine(
        self, process: BlackScholesMertonProcess, steps: int = ...
    ) -> None: ...
    def set_fd_pricing_engine(
        self,
        process: BlackScholesMertonProcess,
        t_grid: int = ...,
        x_grid: int = ...,
        damping_steps: int = ...,
    ) -> None: ...

class OvernightIndexedSwap:
    def NPV(self) -> float: ...
    def fair_rate(self) -> float: ...
    def fair_spread(self) -> float: ...
    def fixed_leg_NPV(self) -> float: ...
    def overnight_leg_NPV(self) -> float: ...
    def set_pricing_engine(self, discount_curve: YieldTermStructureHandle) -> None: ...

class ForwardRateAgreement:
    def __init__(
        self,
        index: IborIndex,
        value_date: Date,
        type: Position,
        strike_forward_rate: float,
        notional_amount: float,
        discount_curve: YieldTermStructureHandle = ...,
    ) -> None: ...
    def __init__(
        self,
        index: IborIndex,
        value_date: Date,
        maturity_date: Date,
        type: Position,
        strike_forward_rate: float,
        notional_amount: float,
        discount_curve: YieldTermStructureHandle = ...,
    ) -> None: ...  # type: ignore[misc]
    def NPV(self) -> float: ...
    def amount(self) -> float: ...
    def forward_rate(self) -> InterestRate: ...
    def fixing_date(self) -> Date: ...

class BarrierOption:
    def __init__(
        self,
        barrier_type: BarrierType,
        barrier: float,
        rebate: float,
        payoff: PlainVanillaPayoff,
        exercise: EuropeanExercise,
    ) -> None: ...
    def NPV(self) -> float: ...
    def delta(self) -> float: ...
    def gamma(self) -> float: ...
    def vega(self) -> float: ...
    def set_pricing_engine(self, process: BlackScholesMertonProcess) -> None: ...

class CapFloor:
    def __init__(
        self,
        type: CapFloorType,
        schedule: Schedule,
        index: IborIndex,
        strike: float,
        nominal: float = ...,
        fixing_days: int = ...,
    ) -> None: ...
    def NPV(self) -> float: ...
    def atm_rate(self, discount_curve: YieldTermStructureHandle) -> float: ...
    def start_date(self) -> Date: ...
    def maturity_date(self) -> Date: ...
    def type(self) -> CapFloorType: ...
    def set_pricing_engine(
        self,
        discount_curve: YieldTermStructureHandle,
        volatility: float,
        day_counter: DayCounter = ...,
        displacement: float = ...,
    ) -> None: ...

class Swaption:
    def __init__(
        self,
        swap: VanillaSwap,
        exercise: EuropeanExercise,
        delivery: SettlementType = ...,
        settlement_method: SettlementMethod = ...,
    ) -> None: ...
    def NPV(self) -> float: ...
    def type(self) -> SwapType: ...
    def settlement_type(self) -> SettlementType: ...
    def settlement_method(self) -> SettlementMethod: ...
    def is_expired(self) -> bool: ...
    def set_pricing_engine(
        self,
        discount_curve: YieldTermStructureHandle,
        volatility: float,
        day_counter: DayCounter = ...,
        displacement: float = ...,
    ) -> None: ...

def set_evaluation_date(date: Date) -> None: ...
def get_evaluation_date() -> Date: ...
def make_quote_handle(value: float) -> QuoteHandle: ...
def make_vanilla_swap(
    swap_tenor: Period,
    index: IborIndex,
    fixed_rate: float,
    effective_date: Date,
    fixed_leg_tenor: Period = ...,
    fixed_day_count: DayCounter = ...,
    type: SwapType = ...,
    nominal: float = ...,
    floating_spread: float = ...,
) -> VanillaSwap: ...
def make_ois(
    swap_tenor: Period,
    overnight_index: OvernightIndex,
    fixed_rate: float,
    forward_start: Period = ...,
    type: SwapType = ...,
    nominal: float = ...,
    overnight_spread: float = ...,
) -> OvernightIndexedSwap: ...
def discount_times(
    curve_handle: YieldTermStructureHandle,
    times: object,
    extrapolate: bool = ...,
) -> object: ...
def discount_dates(
    curve_handle: YieldTermStructureHandle,
    dates: Sequence[Date],
    extrapolate: bool = ...,
) -> object: ...
def BlackSwaptionEngine(
    discount_curve: YieldTermStructureHandle,
) -> YieldTermStructureHandle: ...
def AnalyticEuropeanEngine(
    process: BlackScholesMertonProcess,
) -> BlackScholesMertonProcess: ...
def BaroneAdesiWhaleyEngine(
    process: BlackScholesMertonProcess,
) -> BlackScholesMertonProcess: ...
def AnalyticBarrierEngine(
    process: BlackScholesMertonProcess,
) -> BlackScholesMertonProcess: ...
def BlackConstantVol(
    reference_date: Date,
    calendar: Calendar,
    volatility: float,
    day_counter: DayCounter,
) -> object: ...
def make_cap(
    tenor: Period,
    index: IborIndex,
    strike: float,
    nominal: float = ...,
    forward_start: Period = ...,
) -> CapFloor: ...
def make_floor(
    tenor: Period,
    index: IborIndex,
    strike: float,
    nominal: float = ...,
    forward_start: Period = ...,
) -> CapFloor: ...
def BlackCapFloorEngine(
    discount_curve: YieldTermStructureHandle,
    volatility: float,
    day_counter: DayCounter = ...,
    displacement: float = ...,
) -> YieldTermStructureHandle: ...
def simulate_gbm_paths(
    process: BlackScholesMertonProcess,
    length: float,
    time_steps: int,
    samples: int,
    seed: int = ...,
) -> object: ...
def Actual365Fixed() -> DayCounter: ...
def Actual360() -> DayCounter: ...
def ActualActual(convention: ActualActualConvention = ...) -> DayCounter: ...
def Thirty360(convention: Thirty360Convention = ...) -> DayCounter: ...
def TARGET() -> Calendar: ...
def NullCalendar() -> Calendar: ...
def WeekendsOnly() -> Calendar: ...
def UnitedKingdom() -> Calendar: ...
def Japan() -> Calendar: ...
def Germany(market: GermanyMarket = ...) -> Calendar: ...
def UnitedStates(market: UnitedStatesMarket = ...) -> Calendar: ...
def FlatForward(
    reference_date: Date, forward: float | QuoteHandle, day_counter: DayCounter
) -> YieldTermStructureHandle: ...
def DepositRateHelper(
    rate: float | QuoteHandle,
    tenor: Period,
    fixing_days: int,
    calendar: Calendar,
    convention: BusinessDayConvention,
    end_of_month: bool,
    day_counter: DayCounter,
) -> RateHelper: ...
def FraRateHelper(
    rate: float | QuoteHandle,
    months_to_start: int,
    months_to_end: int = ...,
    fixing_days: int = ...,
    calendar: Calendar = ...,
    convention: BusinessDayConvention = ...,
    end_of_month: bool = ...,
    day_counter: DayCounter = ...,
) -> RateHelper: ...
@overload
def FraRateHelper(
    rate: QuoteHandle,
    months_to_start: int,
    ibor_index: IborIndex,
) -> RateHelper: ...
def SwapRateHelper(
    rate: float | QuoteHandle,
    tenor: Period,
    calendar: Calendar,
    fixed_frequency: Frequency,
    fixed_convention: BusinessDayConvention,
    fixed_day_count: DayCounter,
    ibor_index: IborIndex,
) -> RateHelper: ...
def PiecewiseLogLinearDiscountCurve(
    reference_date: Date,
    helpers: Iterable[RateHelper],
    day_counter: DayCounter,
) -> YieldTermStructureHandle: ...
@overload
def Euribor3M() -> IborIndex: ...
@overload
def Euribor3M(handle: YieldTermStructureHandle) -> IborIndex: ...
@overload
def Euribor6M() -> IborIndex: ...
@overload
def Euribor6M(handle: YieldTermStructureHandle) -> IborIndex: ...
@overload
def Sofr() -> OvernightIndex: ...
@overload
def Sofr(handle: YieldTermStructureHandle) -> OvernightIndex: ...
@overload
def Estr() -> OvernightIndex: ...
@overload
def Estr(handle: YieldTermStructureHandle) -> OvernightIndex: ...
@overload
def Eonia() -> OvernightIndex: ...
@overload
def Eonia(handle: YieldTermStructureHandle) -> OvernightIndex: ...

__version__: str
