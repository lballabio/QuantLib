# This file was created automatically by SWIG.
import QuantLibc
class BoundaryConditionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_BoundaryCondition(self.this)
    def type(self):
        val = QuantLibc.BoundaryCondition_type(self.this)
        return val
    def value(self):
        val = QuantLibc.BoundaryCondition_value(self.this)
        return val
    def __repr__(self):
        return "<C BoundaryCondition instance>"
class BoundaryCondition(BoundaryConditionPtr):
    def __init__(self,arg0,arg1) :
        self.this = QuantLibc.new_BoundaryCondition(arg0,arg1)
        self.thisown = 1




class DatePtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Date(self.this)
    def weekday(self):
        val = QuantLibc.Date_weekday(self.this)
        return val
    def dayOfMonth(self):
        val = QuantLibc.Date_dayOfMonth(self.this)
        return val
    def dayOfYear(self):
        val = QuantLibc.Date_dayOfYear(self.this)
        return val
    def month(self):
        val = QuantLibc.Date_month(self.this)
        return val
    def year(self):
        val = QuantLibc.Date_year(self.this)
        return val
    def serialNumber(self):
        val = QuantLibc.Date_serialNumber(self.this)
        return val
    def plusDays(self,arg0):
        val = QuantLibc.Date_plusDays(self.this,arg0)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def plusWeeks(self,arg0):
        val = QuantLibc.Date_plusWeeks(self.this,arg0)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def plusMonths(self,arg0):
        val = QuantLibc.Date_plusMonths(self.this,arg0)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def plusYears(self,arg0):
        val = QuantLibc.Date_plusYears(self.this,arg0)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def plus(self,arg0,arg1):
        val = QuantLibc.Date_plus(self.this,arg0,arg1)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def monthNumber(self):
        val = QuantLibc.Date_monthNumber(self.this)
        return val
    def weekdayNumber(self):
        val = QuantLibc.Date_weekdayNumber(self.this)
        return val
    def __add__(self,arg0):
        val = QuantLibc.Date___add__(self.this,arg0)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def __sub__(self,arg0):
        val = QuantLibc.Date___sub__(self.this,arg0)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.Date___cmp__(self.this,arg0.this)
        return val
    def __str__(self):
        val = QuantLibc.Date___str__(self.this)
        return val
    def __repr__(self):
        val = QuantLibc.Date___repr__(self.this)
        return val
class Date(DatePtr):
    def __init__(self,arg0,arg1,arg2) :
        self.this = QuantLibc.new_Date(arg0,arg1,arg2)
        self.thisown = 1




class CalendarPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Calendar(self.this)
    def isBusinessDay(self,arg0):
        val = QuantLibc.Calendar_isBusinessDay(self.this,arg0.this)
        return val
    def isHoliday(self,arg0):
        val = QuantLibc.Calendar_isHoliday(self.this,arg0.this)
        return val
    def roll(self,arg0,*args):
        val = apply(QuantLibc.Calendar_roll,(self.this,arg0.this,)+args)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def advance(self,arg0,arg1,arg2,*args):
        val = apply(QuantLibc.Calendar_advance,(self.this,arg0.this,arg1,arg2,)+args)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def __str__(self):
        val = QuantLibc.Calendar___str__(self.this)
        return val
    def __repr__(self):
        val = QuantLibc.Calendar___repr__(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.Calendar___cmp__(self.this,arg0.this)
        return val
    def __nonzero__(self):
        val = QuantLibc.Calendar___nonzero__(self.this)
        return val
class Calendar(CalendarPtr):
    def __init__(self,this):
        self.this = this




class CurrencyPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Currency(self.this)
    def settlementCalendar(self):
        val = QuantLibc.Currency_settlementCalendar(self.this)
        val = CalendarPtr(val)
        val.thisown = 1
        return val
    def settlementDays(self):
        val = QuantLibc.Currency_settlementDays(self.this)
        return val
    def settlementDate(self,arg0):
        val = QuantLibc.Currency_settlementDate(self.this,arg0.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def __str__(self):
        val = QuantLibc.Currency___str__(self.this)
        return val
    def __repr__(self):
        val = QuantLibc.Currency___repr__(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.Currency___cmp__(self.this,arg0.this)
        return val
    def __nonzero__(self):
        val = QuantLibc.Currency___nonzero__(self.this)
        return val
class Currency(CurrencyPtr):
    def __init__(self,this):
        self.this = this




class DayCounterPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DayCounter(self.this)
    def dayCount(self,arg0,arg1):
        val = QuantLibc.DayCounter_dayCount(self.this,arg0.this,arg1.this)
        return val
    def yearFraction(self,arg0,arg1):
        val = QuantLibc.DayCounter_yearFraction(self.this,arg0.this,arg1.this)
        return val
    def __str__(self):
        val = QuantLibc.DayCounter___str__(self.this)
        return val
    def __repr__(self):
        val = QuantLibc.DayCounter___repr__(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.DayCounter___cmp__(self.this,arg0.this)
        return val
    def __nonzero__(self):
        val = QuantLibc.DayCounter___nonzero__(self.this)
        return val
class DayCounter(DayCounterPtr):
    def __init__(self,this):
        self.this = this




class NormalDistributionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_NormalDistribution(self.this)
    def derivative(self,arg0):
        val = QuantLibc.NormalDistribution_derivative(self.this,arg0)
        return val
    def __call__(self,arg0):
        val = QuantLibc.NormalDistribution___call__(self.this,arg0)
        return val
    def __repr__(self):
        return "<C NormalDistribution instance>"
class NormalDistribution(NormalDistributionPtr):
    def __init__(self,*args) :
        self.this = apply(QuantLibc.new_NormalDistribution,()+args)
        self.thisown = 1




class CumulativeNormalDistributionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_CumulativeNormalDistribution(self.this)
    def derivative(self,arg0):
        val = QuantLibc.CumulativeNormalDistribution_derivative(self.this,arg0)
        return val
    def __call__(self,arg0):
        val = QuantLibc.CumulativeNormalDistribution___call__(self.this,arg0)
        return val
    def __repr__(self):
        return "<C CumulativeNormalDistribution instance>"
class CumulativeNormalDistribution(CumulativeNormalDistributionPtr):
    def __init__(self,*args) :
        self.this = apply(QuantLibc.new_CumulativeNormalDistribution,()+args)
        self.thisown = 1




class InvCumulativeNormalDistributionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_InvCumulativeNormalDistribution(self.this)
    def __call__(self,arg0):
        val = QuantLibc.InvCumulativeNormalDistribution___call__(self.this,arg0)
        return val
    def __repr__(self):
        return "<C InvCumulativeNormalDistribution instance>"
class InvCumulativeNormalDistribution(InvCumulativeNormalDistributionPtr):
    def __init__(self,*args) :
        self.this = apply(QuantLibc.new_InvCumulativeNormalDistribution,()+args)
        self.thisown = 1




class ArrayPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Array(self.this)
    def size(self):
        val = QuantLibc.Array_size(self.this)
        return val
    def __getitem__(self,arg0):
        val = QuantLibc.Array___getitem__(self.this,arg0)
        return val
    def __setitem__(self,arg0,arg1):
        val = QuantLibc.Array___setitem__(self.this,arg0,arg1)
        return val
    def __str__(self):
        val = QuantLibc.Array___str__(self.this)
        return val
    def __repr__(self):
        return "<C Array instance>"
class Array(ArrayPtr):
    def __init__(self,arg0,*args) :
        self.this = apply(QuantLibc.new_Array,(arg0,)+args)
        self.thisown = 1




class ArrayLexicographicalViewPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_ArrayLexicographicalView(self.this)
    def xSize(self):
        val = QuantLibc.ArrayLexicographicalView_xSize(self.this)
        return val
    def ySize(self):
        val = QuantLibc.ArrayLexicographicalView_ySize(self.this)
        return val
    def __getitem__(self,arg0):
        val = QuantLibc.ArrayLexicographicalView___getitem__(self.this,arg0)
        val = ArrayLexicographicalViewColumnPtr(val)
        val.thisown = 1
        return val
    def __str__(self):
        val = QuantLibc.ArrayLexicographicalView___str__(self.this)
        return val
    def __repr__(self):
        return "<C ArrayLexicographicalView instance>"
class ArrayLexicographicalView(ArrayLexicographicalViewPtr):
    def __init__(self,this):
        self.this = this




class ArrayLexicographicalViewColumnPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_ArrayLexicographicalViewColumn(self.this)
    def __getitem__(self,arg0):
        val = QuantLibc.ArrayLexicographicalViewColumn___getitem__(self.this,arg0)
        return val
    def __setitem__(self,arg0,arg1):
        val = QuantLibc.ArrayLexicographicalViewColumn___setitem__(self.this,arg0,arg1)
        return val
    def __repr__(self):
        return "<C ArrayLexicographicalViewColumn instance>"
class ArrayLexicographicalViewColumn(ArrayLexicographicalViewColumnPtr):
    def __init__(self,this):
        self.this = this




class HistoryIteratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryIterator(self.this)
    def date(self):
        val = QuantLibc.HistoryIterator_date(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def value(self):
        val = QuantLibc.HistoryIterator_value(self.this)
        return val
    def advance(self):
        val = QuantLibc.HistoryIterator_advance(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.HistoryIterator___cmp__(self.this,arg0.this)
        return val
    def __str__(self):
        val = QuantLibc.HistoryIterator___str__(self.this)
        return val
    def __repr__(self):
        return "<C HistoryIterator instance>"
class HistoryIterator(HistoryIteratorPtr):
    def __init__(self,this):
        self.this = this




class HistoryValidIteratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryValidIterator(self.this)
    def date(self):
        val = QuantLibc.HistoryValidIterator_date(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def value(self):
        val = QuantLibc.HistoryValidIterator_value(self.this)
        return val
    def advance(self):
        val = QuantLibc.HistoryValidIterator_advance(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.HistoryValidIterator___cmp__(self.this,arg0.this)
        return val
    def __str__(self):
        val = QuantLibc.HistoryValidIterator___str__(self.this)
        return val
    def __repr__(self):
        return "<C HistoryValidIterator instance>"
class HistoryValidIterator(HistoryValidIteratorPtr):
    def __init__(self,this):
        self.this = this




class HistoryDataIteratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryDataIterator(self.this)
    def __float__(self):
        val = QuantLibc.HistoryDataIterator___float__(self.this)
        return val
    def advance(self):
        val = QuantLibc.HistoryDataIterator_advance(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.HistoryDataIterator___cmp__(self.this,arg0.this)
        return val
    def __str__(self):
        val = QuantLibc.HistoryDataIterator___str__(self.this)
        return val
    def __repr__(self):
        return "<C HistoryDataIterator instance>"
class HistoryDataIterator(HistoryDataIteratorPtr):
    def __init__(self,this):
        self.this = this




class HistoryValidDataIteratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryValidDataIterator(self.this)
    def __float__(self):
        val = QuantLibc.HistoryValidDataIterator___float__(self.this)
        return val
    def advance(self):
        val = QuantLibc.HistoryValidDataIterator_advance(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.HistoryValidDataIterator___cmp__(self.this,arg0.this)
        return val
    def __str__(self):
        val = QuantLibc.HistoryValidDataIterator___str__(self.this)
        return val
    def __repr__(self):
        return "<C HistoryValidDataIterator instance>"
class HistoryValidDataIterator(HistoryValidDataIteratorPtr):
    def __init__(self,this):
        self.this = this




class HistoryPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_History(self.this)
    def firstDate(self):
        val = QuantLibc.History_firstDate(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def lastDate(self):
        val = QuantLibc.History_lastDate(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def size(self):
        val = QuantLibc.History_size(self.this)
        return val
    def begin(self):
        val = QuantLibc.History_begin(self.this)
        val = HistoryIteratorPtr(val)
        val.thisown = 1
        return val
    def end(self):
        val = QuantLibc.History_end(self.this)
        val = HistoryIteratorPtr(val)
        val.thisown = 1
        return val
    def iterator(self,arg0):
        val = QuantLibc.History_iterator(self.this,arg0.this)
        val = HistoryIteratorPtr(val)
        val.thisown = 1
        return val
    def vbegin(self):
        val = QuantLibc.History_vbegin(self.this)
        val = HistoryValidIteratorPtr(val)
        val.thisown = 1
        return val
    def vend(self):
        val = QuantLibc.History_vend(self.this)
        val = HistoryValidIteratorPtr(val)
        val.thisown = 1
        return val
    def valid_iterator(self,arg0):
        val = QuantLibc.History_valid_iterator(self.this,arg0.this)
        val = HistoryValidIteratorPtr(val)
        val.thisown = 1
        return val
    def dbegin(self):
        val = QuantLibc.History_dbegin(self.this)
        val = HistoryDataIteratorPtr(val)
        val.thisown = 1
        return val
    def dend(self):
        val = QuantLibc.History_dend(self.this)
        val = HistoryDataIteratorPtr(val)
        val.thisown = 1
        return val
    def data_iterator(self,arg0):
        val = QuantLibc.History_data_iterator(self.this,arg0.this)
        val = HistoryDataIteratorPtr(val)
        val.thisown = 1
        return val
    def vdbegin(self):
        val = QuantLibc.History_vdbegin(self.this)
        val = HistoryValidDataIteratorPtr(val)
        val.thisown = 1
        return val
    def vdend(self):
        val = QuantLibc.History_vdend(self.this)
        val = HistoryValidDataIteratorPtr(val)
        val.thisown = 1
        return val
    def valid_data_iterator(self,arg0):
        val = QuantLibc.History_valid_data_iterator(self.this,arg0.this)
        val = HistoryValidDataIteratorPtr(val)
        val.thisown = 1
        return val
    def __getitem__(self,arg0):
        val = QuantLibc.History___getitem__(self.this,arg0.this)
        return val
    def __str__(self):
        val = QuantLibc.History___str__(self.this)
        return val
    def __repr__(self):
        val = QuantLibc.History___repr__(self.this)
        return val
class History(HistoryPtr):
    def __init__(self,arg0,arg1) :
        self.this = QuantLibc.new_History(arg0,arg1)
        self.thisown = 1




class TermStructurePtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_TermStructure(self.this)
    def clone(self):
        val = QuantLibc.TermStructure_clone(self.this)
        val = TermStructurePtr(val)
        val.thisown = 1
        return val
    def currency(self):
        val = QuantLibc.TermStructure_currency(self.this)
        val = CurrencyPtr(val)
        val.thisown = 1
        return val
    def todaysDate(self):
        val = QuantLibc.TermStructure_todaysDate(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def settlementDate(self):
        val = QuantLibc.TermStructure_settlementDate(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def calendar(self):
        val = QuantLibc.TermStructure_calendar(self.this)
        val = CalendarPtr(val)
        val.thisown = 1
        return val
    def maxDate(self):
        val = QuantLibc.TermStructure_maxDate(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def minDate(self):
        val = QuantLibc.TermStructure_minDate(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def zeroYield(self,arg0):
        val = QuantLibc.TermStructure_zeroYield(self.this,arg0.this)
        return val
    def discount(self,arg0):
        val = QuantLibc.TermStructure_discount(self.this,arg0.this)
        return val
    def forward(self,arg0):
        val = QuantLibc.TermStructure_forward(self.this,arg0.this)
        return val
    def __nonzero__(self):
        val = QuantLibc.TermStructure___nonzero__(self.this)
        return val
    def __repr__(self):
        return "<C TermStructure instance>"
class TermStructure(TermStructurePtr):
    def __init__(self,this):
        self.this = this




class DepositPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Deposit(self.this)
    def maturity(self):
        val = QuantLibc.Deposit_maturity(self.this)
        val = DatePtr(val)
        val.thisown = 1
        return val
    def rate(self):
        val = QuantLibc.Deposit_rate(self.this)
        return val
    def dayCounter(self):
        val = QuantLibc.Deposit_dayCounter(self.this)
        val = DayCounterPtr(val)
        val.thisown = 1
        return val
    def __repr__(self):
        return "<C Deposit instance>"
class Deposit(DepositPtr):
    def __init__(self,arg0,arg1,arg2) :
        self.this = QuantLibc.new_Deposit(arg0.this,arg1,arg2.this)
        self.thisown = 1




class InstrumentPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Instrument(self.this)
    def setPrice(self,arg0):
        val = QuantLibc.Instrument_setPrice(self.this,arg0)
        return val
    def setTermStructure(self,arg0):
        val = QuantLibc.Instrument_setTermStructure(self.this,arg0.this)
        return val
    def isinCode(self):
        val = QuantLibc.Instrument_isinCode(self.this)
        return val
    def description(self):
        val = QuantLibc.Instrument_description(self.this)
        return val
    def termStructure(self):
        val = QuantLibc.Instrument_termStructure(self.this)
        val = TermStructurePtr(val)
        val.thisown = 1
        return val
    def NPV(self):
        val = QuantLibc.Instrument_NPV(self.this)
        return val
    def price(self):
        val = QuantLibc.Instrument_price(self.this)
        return val
    def __str__(self):
        val = QuantLibc.Instrument___str__(self.this)
        return val
    def __repr__(self):
        val = QuantLibc.Instrument___repr__(self.this)
        return val
    def __cmp__(self,arg0):
        val = QuantLibc.Instrument___cmp__(self.this,arg0.this)
        return val
    def __nonzero__(self):
        val = QuantLibc.Instrument___nonzero__(self.this)
        return val
class Instrument(InstrumentPtr):
    def __init__(self,this):
        self.this = this




class InterpolationPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Interpolation(self.this)
    def __call__(self,arg0):
        val = QuantLibc.Interpolation___call__(self.this,arg0)
        return val
    def __nonzero__(self):
        val = QuantLibc.Interpolation___nonzero__(self.this)
        return val
    def __repr__(self):
        return "<C Interpolation instance>"
class Interpolation(InterpolationPtr):
    def __init__(self,this):
        self.this = this




class StandardPathGeneratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def next(self):
        val = QuantLibc.StandardPathGenerator_next(self.this)
        return val
    def weight(self):
        val = QuantLibc.StandardPathGenerator_weight(self.this)
        return val
    def __repr__(self):
        return "<C StandardPathGenerator instance>"
class StandardPathGenerator(StandardPathGeneratorPtr):
    def __init__(self,arg0,*args) :
        self.this = apply(QuantLibc.new_StandardPathGenerator,(arg0,)+args)
        self.thisown = 1




class McEuropeanPricerPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def value(self):
        val = QuantLibc.McEuropeanPricer_value(self.this)
        return val
    def errorEstimate(self):
        val = QuantLibc.McEuropeanPricer_errorEstimate(self.this)
        return val
    def __repr__(self):
        return "<C McEuropeanPricer instance>"
class McEuropeanPricer(McEuropeanPricerPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) :
        self.this = QuantLibc.new_McEuropeanPricer(arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)
        self.thisown = 1




class GeometricAsianOptionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def value(self):
        val = QuantLibc.GeometricAsianOption_value(self.this)
        return val
    def __repr__(self):
        return "<C GeometricAsianOption instance>"
class GeometricAsianOption(GeometricAsianOptionPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6) :
        self.this = QuantLibc.new_GeometricAsianOption(arg0,arg1,arg2,arg3,arg4,arg5,arg6)
        self.thisown = 1




class McAsianPricerPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def value(self):
        val = QuantLibc.McAsianPricer_value(self.this)
        return val
    def errorEstimate(self):
        val = QuantLibc.McAsianPricer_errorEstimate(self.this)
        return val
    def __repr__(self):
        return "<C McAsianPricer instance>"
class McAsianPricer(McAsianPricerPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) :
        self.this = QuantLibc.new_McAsianPricer(arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)
        self.thisown = 1




class TridiagonalOperatorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_TridiagonalOperator(self.this)
    def solveFor(self,arg0):
        val = QuantLibc.TridiagonalOperator_solveFor(self.this,arg0)
        return val
    def applyTo(self,arg0):
        val = QuantLibc.TridiagonalOperator_applyTo(self.this,arg0)
        return val
    def size(self):
        val = QuantLibc.TridiagonalOperator_size(self.this)
        return val
    def setLowerBC(self,arg0):
        val = QuantLibc.TridiagonalOperator_setLowerBC(self.this,arg0.this)
        return val
    def setHigherBC(self,arg0):
        val = QuantLibc.TridiagonalOperator_setHigherBC(self.this,arg0.this)
        return val
    def setFirstRow(self,arg0,arg1):
        val = QuantLibc.TridiagonalOperator_setFirstRow(self.this,arg0,arg1)
        return val
    def setMidRow(self,arg0,arg1,arg2,arg3):
        val = QuantLibc.TridiagonalOperator_setMidRow(self.this,arg0,arg1,arg2,arg3)
        return val
    def setMidRows(self,arg0,arg1,arg2):
        val = QuantLibc.TridiagonalOperator_setMidRows(self.this,arg0,arg1,arg2)
        return val
    def setLastRow(self,arg0,arg1):
        val = QuantLibc.TridiagonalOperator_setLastRow(self.this,arg0,arg1)
        return val
    def __repr__(self):
        return "<C TridiagonalOperator instance>"
class TridiagonalOperator(TridiagonalOperatorPtr):
    def __init__(self,arg0,arg1,arg2) :
        self.this = QuantLibc.new_TridiagonalOperator(arg0,arg1,arg2)
        self.thisown = 1




class DPlusPtr(TridiagonalOperatorPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DPlus(self.this)
    def __repr__(self):
        return "<C DPlus instance>"
class DPlus(DPlusPtr):
    def __init__(self,arg0,arg1) :
        self.this = QuantLibc.new_DPlus(arg0,arg1)
        self.thisown = 1




class DMinusPtr(TridiagonalOperatorPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DMinus(self.this)
    def __repr__(self):
        return "<C DMinus instance>"
class DMinus(DMinusPtr):
    def __init__(self,arg0,arg1) :
        self.this = QuantLibc.new_DMinus(arg0,arg1)
        self.thisown = 1




class DZeroPtr(TridiagonalOperatorPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DZero(self.this)
    def __repr__(self):
        return "<C DZero instance>"
class DZero(DZeroPtr):
    def __init__(self,arg0,arg1) :
        self.this = QuantLibc.new_DZero(arg0,arg1)
        self.thisown = 1




class DPlusDMinusPtr(TridiagonalOperatorPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DPlusDMinus(self.this)
    def __repr__(self):
        return "<C DPlusDMinus instance>"
class DPlusDMinus(DPlusDMinusPtr):
    def __init__(self,arg0,arg1) :
        self.this = QuantLibc.new_DPlusDMinus(arg0,arg1)
        self.thisown = 1




class BSMEuropeanOptionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_BSMEuropeanOption(self.this)
    def setVolatility(self,arg0):
        val = QuantLibc.BSMEuropeanOption_setVolatility(self.this,arg0)
        return val
    def setRiskFreeRate(self,arg0):
        val = QuantLibc.BSMEuropeanOption_setRiskFreeRate(self.this,arg0)
        return val
    def value(self):
        val = QuantLibc.BSMEuropeanOption_value(self.this)
        return val
    def delta(self):
        val = QuantLibc.BSMEuropeanOption_delta(self.this)
        return val
    def gamma(self):
        val = QuantLibc.BSMEuropeanOption_gamma(self.this)
        return val
    def theta(self):
        val = QuantLibc.BSMEuropeanOption_theta(self.this)
        return val
    def vega(self):
        val = QuantLibc.BSMEuropeanOption_vega(self.this)
        return val
    def rho(self):
        val = QuantLibc.BSMEuropeanOption_rho(self.this)
        return val
    def impliedVolatility(self,arg0,*args):
        val = apply(QuantLibc.BSMEuropeanOption_impliedVolatility,(self.this,arg0,)+args)
        return val
    def __repr__(self):
        return "<C BSMEuropeanOption instance>"
class BSMEuropeanOption(BSMEuropeanOptionPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6) :
        self.this = QuantLibc.new_BSMEuropeanOption(arg0,arg1,arg2,arg3,arg4,arg5,arg6)
        self.thisown = 1




class BSMAmericanOptionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_BSMAmericanOption(self.this)
    def setVolatility(self,arg0):
        val = QuantLibc.BSMAmericanOption_setVolatility(self.this,arg0)
        return val
    def setRiskFreeRate(self,arg0):
        val = QuantLibc.BSMAmericanOption_setRiskFreeRate(self.this,arg0)
        return val
    def value(self):
        val = QuantLibc.BSMAmericanOption_value(self.this)
        return val
    def delta(self):
        val = QuantLibc.BSMAmericanOption_delta(self.this)
        return val
    def gamma(self):
        val = QuantLibc.BSMAmericanOption_gamma(self.this)
        return val
    def theta(self):
        val = QuantLibc.BSMAmericanOption_theta(self.this)
        return val
    def vega(self):
        val = QuantLibc.BSMAmericanOption_vega(self.this)
        return val
    def rho(self):
        val = QuantLibc.BSMAmericanOption_rho(self.this)
        return val
    def impliedVolatility(self,arg0,*args):
        val = apply(QuantLibc.BSMAmericanOption_impliedVolatility,(self.this,arg0,)+args)
        return val
    def __repr__(self):
        return "<C BSMAmericanOption instance>"
class BSMAmericanOption(BSMAmericanOptionPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6,*args) :
        self.this = apply(QuantLibc.new_BSMAmericanOption,(arg0,arg1,arg2,arg3,arg4,arg5,arg6,)+args)
        self.thisown = 1




class DividendAmericanOptionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DividendAmericanOption(self.this)
    def value(self):
        val = QuantLibc.DividendAmericanOption_value(self.this)
        return val
    def delta(self):
        val = QuantLibc.DividendAmericanOption_delta(self.this)
        return val
    def gamma(self):
        val = QuantLibc.DividendAmericanOption_gamma(self.this)
        return val
    def theta(self):
        val = QuantLibc.DividendAmericanOption_theta(self.this)
        return val
    def vega(self):
        val = QuantLibc.DividendAmericanOption_vega(self.this)
        return val
    def rho(self):
        val = QuantLibc.DividendAmericanOption_rho(self.this)
        return val
    def impliedVolatility(self,arg0,*args):
        val = apply(QuantLibc.DividendAmericanOption_impliedVolatility,(self.this,arg0,)+args)
        return val
    def __repr__(self):
        return "<C DividendAmericanOption instance>"
class DividendAmericanOption(DividendAmericanOptionPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,*args) :
        self.this = apply(QuantLibc.new_DividendAmericanOption,(arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,)+args)
        self.thisown = 1




class DividendEuropeanOptionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_DividendEuropeanOption(self.this)
    def value(self):
        val = QuantLibc.DividendEuropeanOption_value(self.this)
        return val
    def delta(self):
        val = QuantLibc.DividendEuropeanOption_delta(self.this)
        return val
    def gamma(self):
        val = QuantLibc.DividendEuropeanOption_gamma(self.this)
        return val
    def theta(self):
        val = QuantLibc.DividendEuropeanOption_theta(self.this)
        return val
    def vega(self):
        val = QuantLibc.DividendEuropeanOption_vega(self.this)
        return val
    def rho(self):
        val = QuantLibc.DividendEuropeanOption_rho(self.this)
        return val
    def impliedVolatility(self,arg0,*args):
        val = apply(QuantLibc.DividendEuropeanOption_impliedVolatility,(self.this,arg0,)+args)
        return val
    def __repr__(self):
        return "<C DividendEuropeanOption instance>"
class DividendEuropeanOption(DividendEuropeanOptionPtr):
    def __init__(self,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) :
        self.this = QuantLibc.new_DividendEuropeanOption(arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
        self.thisown = 1




class UniformRandomGeneratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_UniformRandomGenerator(self.this)
    def next(self):
        val = QuantLibc.UniformRandomGenerator_next(self.this)
        return val
    def weight(self):
        val = QuantLibc.UniformRandomGenerator_weight(self.this)
        return val
    def __repr__(self):
        return "<C UniformRandomGenerator instance>"
class UniformRandomGenerator(UniformRandomGeneratorPtr):
    def __init__(self,*args) :
        self.this = apply(QuantLibc.new_UniformRandomGenerator,()+args)
        self.thisown = 1




class GaussianRandomGeneratorPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_GaussianRandomGenerator(self.this)
    def next(self):
        val = QuantLibc.GaussianRandomGenerator_next(self.this)
        return val
    def weight(self):
        val = QuantLibc.GaussianRandomGenerator_weight(self.this)
        return val
    def __repr__(self):
        return "<C GaussianRandomGenerator instance>"
class GaussianRandomGenerator(GaussianRandomGeneratorPtr):
    def __init__(self,*args) :
        self.this = apply(QuantLibc.new_GaussianRandomGenerator,()+args)
        self.thisown = 1




class RiskToolPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_RiskTool(self.this)
    def samples(self):
        val = QuantLibc.RiskTool_samples(self.this)
        return val
    def weightSum(self):
        val = QuantLibc.RiskTool_weightSum(self.this)
        return val
    def mean(self):
        val = QuantLibc.RiskTool_mean(self.this)
        return val
    def variance(self):
        val = QuantLibc.RiskTool_variance(self.this)
        return val
    def standardDeviation(self):
        val = QuantLibc.RiskTool_standardDeviation(self.this)
        return val
    def errorEstimate(self):
        val = QuantLibc.RiskTool_errorEstimate(self.this)
        return val
    def skewness(self):
        val = QuantLibc.RiskTool_skewness(self.this)
        return val
    def kurtosis(self):
        val = QuantLibc.RiskTool_kurtosis(self.this)
        return val
    def min(self):
        val = QuantLibc.RiskTool_min(self.this)
        return val
    def max(self):
        val = QuantLibc.RiskTool_max(self.this)
        return val
    def valueAtRisk(self,arg0):
        val = QuantLibc.RiskTool_valueAtRisk(self.this,arg0)
        return val
    def shortfall(self,arg0):
        val = QuantLibc.RiskTool_shortfall(self.this,arg0)
        return val
    def averageShortfall(self,arg0):
        val = QuantLibc.RiskTool_averageShortfall(self.this,arg0)
        return val
    def add(self,arg0,*args):
        val = apply(QuantLibc.RiskTool_add,(self.this,arg0,)+args)
        return val
    def reset(self):
        val = QuantLibc.RiskTool_reset(self.this)
        return val
    def addSequence(self,arg0):
        val = QuantLibc.RiskTool_addSequence(self.this,arg0)
        return val
    def addWeightedSequence(self,arg0,arg1):
        val = QuantLibc.RiskTool_addWeightedSequence(self.this,arg0,arg1)
        return val
    def __repr__(self):
        return "<C RiskTool instance>"
class RiskTool(RiskToolPtr):
    def __init__(self) :
        self.this = QuantLibc.new_RiskTool()
        self.thisown = 1




class ObjectiveFunctionPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def value(self,arg0):
        val = QuantLibc.ObjectiveFunction_value(self.this,arg0)
        return val
    def derivative(self,arg0):
        val = QuantLibc.ObjectiveFunction_derivative(self.this,arg0)
        return val
    def __repr__(self):
        return "<C ObjectiveFunction instance>"
class ObjectiveFunction(ObjectiveFunctionPtr):
    def __init__(self,this):
        self.this = this




class Solver1DPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def solve(self,arg0,arg1,arg2,arg3):
        val = QuantLibc.Solver1D_solve(self.this,arg0.this,arg1,arg2,arg3)
        return val
    def bracketedSolve(self,arg0,arg1,arg2,arg3,arg4):
        val = QuantLibc.Solver1D_bracketedSolve(self.this,arg0.this,arg1,arg2,arg3,arg4)
        return val
    def setMaxEvaluations(self,arg0):
        val = QuantLibc.Solver1D_setMaxEvaluations(self.this,arg0)
        return val
    def pySolve(self,arg0,arg1,arg2,arg3):
        val = QuantLibc.Solver1D_pySolve(self.this,arg0,arg1,arg2,arg3)
        return val
    def pyBracketedSolve(self,arg0,arg1,arg2,arg3,arg4):
        val = QuantLibc.Solver1D_pyBracketedSolve(self.this,arg0,arg1,arg2,arg3,arg4)
        return val
    def __repr__(self):
        return "<C Solver1D instance>"
class Solver1D(Solver1DPtr):
    def __init__(self,this):
        self.this = this




class BisectionPtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Bisection(self.this)
    def __repr__(self):
        return "<C Bisection instance>"
class Bisection(BisectionPtr):
    def __init__(self) :
        self.this = QuantLibc.new_Bisection()
        self.thisown = 1




class BrentPtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Brent(self.this)
    def __repr__(self):
        return "<C Brent instance>"
class Brent(BrentPtr):
    def __init__(self) :
        self.this = QuantLibc.new_Brent()
        self.thisown = 1




class FalsePositionPtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_FalsePosition(self.this)
    def __repr__(self):
        return "<C FalsePosition instance>"
class FalsePosition(FalsePositionPtr):
    def __init__(self) :
        self.this = QuantLibc.new_FalsePosition()
        self.thisown = 1




class NewtonPtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Newton(self.this)
    def __repr__(self):
        return "<C Newton instance>"
class Newton(NewtonPtr):
    def __init__(self) :
        self.this = QuantLibc.new_Newton()
        self.thisown = 1




class NewtonSafePtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_NewtonSafe(self.this)
    def __repr__(self):
        return "<C NewtonSafe instance>"
class NewtonSafe(NewtonSafePtr):
    def __init__(self) :
        self.this = QuantLibc.new_NewtonSafe()
        self.thisown = 1




class RidderPtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Ridder(self.this)
    def __repr__(self):
        return "<C Ridder instance>"
class Ridder(RidderPtr):
    def __init__(self) :
        self.this = QuantLibc.new_Ridder()
        self.thisown = 1




class SecantPtr(Solver1DPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Secant(self.this)
    def __repr__(self):
        return "<C Secant instance>"
class Secant(SecantPtr):
    def __init__(self) :
        self.this = QuantLibc.new_Secant()
        self.thisown = 1




class StatisticsPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            QuantLibc.delete_Statistics(self.this)
    def samples(self):
        val = QuantLibc.Statistics_samples(self.this)
        return val
    def weightSum(self):
        val = QuantLibc.Statistics_weightSum(self.this)
        return val
    def mean(self):
        val = QuantLibc.Statistics_mean(self.this)
        return val
    def variance(self):
        val = QuantLibc.Statistics_variance(self.this)
        return val
    def standardDeviation(self):
        val = QuantLibc.Statistics_standardDeviation(self.this)
        return val
    def errorEstimate(self):
        val = QuantLibc.Statistics_errorEstimate(self.this)
        return val
    def skewness(self):
        val = QuantLibc.Statistics_skewness(self.this)
        return val
    def kurtosis(self):
        val = QuantLibc.Statistics_kurtosis(self.this)
        return val
    def min(self):
        val = QuantLibc.Statistics_min(self.this)
        return val
    def max(self):
        val = QuantLibc.Statistics_max(self.this)
        return val
    def add(self,arg0,*args):
        val = apply(QuantLibc.Statistics_add,(self.this,arg0,)+args)
        return val
    def reset(self):
        val = QuantLibc.Statistics_reset(self.this)
        return val
    def addSequence(self,arg0):
        val = QuantLibc.Statistics_addSequence(self.this,arg0)
        return val
    def addWeightedSequence(self,arg0,arg1):
        val = QuantLibc.Statistics_addWeightedSequence(self.this,arg0,arg1)
        return val
    def __repr__(self):
        return "<C Statistics instance>"
class Statistics(StatisticsPtr):
    def __init__(self) :
        self.this = QuantLibc.new_Statistics()
        self.thisown = 1






#-------------- FUNCTION WRAPPERS ------------------

def DateFromSerialNumber(arg0):
    val = QuantLibc.DateFromSerialNumber(arg0)
    val = DatePtr(val)
    val.thisown = 1
    return val

def TARGET():
    val = QuantLibc.TARGET()
    val = CalendarPtr(val)
    val.thisown = 1
    return val

def NewYork():
    val = QuantLibc.NewYork()
    val = CalendarPtr(val)
    val.thisown = 1
    return val

def London():
    val = QuantLibc.London()
    val = CalendarPtr(val)
    val.thisown = 1
    return val

def Frankfurt():
    val = QuantLibc.Frankfurt()
    val = CalendarPtr(val)
    val.thisown = 1
    return val

def Milan():
    val = QuantLibc.Milan()
    val = CalendarPtr(val)
    val.thisown = 1
    return val

def Zurich():
    val = QuantLibc.Zurich()
    val = CalendarPtr(val)
    val.thisown = 1
    return val

def EUR():
    val = QuantLibc.EUR()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def USD():
    val = QuantLibc.USD()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def GBP():
    val = QuantLibc.GBP()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def DEM():
    val = QuantLibc.DEM()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def ITL():
    val = QuantLibc.ITL()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def AUD():
    val = QuantLibc.AUD()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def CAD():
    val = QuantLibc.CAD()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def CHF():
    val = QuantLibc.CHF()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def DKK():
    val = QuantLibc.DKK()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def JPY():
    val = QuantLibc.JPY()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def SEK():
    val = QuantLibc.SEK()
    val = CurrencyPtr(val)
    val.thisown = 1
    return val

def Actual360():
    val = QuantLibc.Actual360()
    val = DayCounterPtr(val)
    val.thisown = 1
    return val

def Actual365():
    val = QuantLibc.Actual365()
    val = DayCounterPtr(val)
    val.thisown = 1
    return val

def Thirty360():
    val = QuantLibc.Thirty360()
    val = DayCounterPtr(val)
    val.thisown = 1
    return val

def Thirty360European():
    val = QuantLibc.Thirty360European()
    val = DayCounterPtr(val)
    val.thisown = 1
    return val

def Thirty360Italian():
    val = QuantLibc.Thirty360Italian()
    val = DayCounterPtr(val)
    val.thisown = 1
    return val

def LexicographicalView(arg0,arg1):
    val = QuantLibc.LexicographicalView(arg0.this,arg1)
    val = ArrayLexicographicalViewPtr(val)
    val.thisown = 1
    return val

def ImpliedTermStructure(arg0,arg1):
    val = QuantLibc.ImpliedTermStructure(arg0.this,arg1.this)
    val = TermStructurePtr(val)
    val.thisown = 1
    return val

def SpreadedTermStructure(arg0,arg1):
    val = QuantLibc.SpreadedTermStructure(arg0.this,arg1)
    val = TermStructurePtr(val)
    val.thisown = 1
    return val

def FlatForward(arg0,arg1,arg2,arg3):
    val = QuantLibc.FlatForward(arg0.this,arg1.this,arg2.this,arg3)
    val = TermStructurePtr(val)
    val.thisown = 1
    return val

def PiecewiseConstantForwards(arg0,arg1,arg2,arg3):
    val = QuantLibc.PiecewiseConstantForwards(arg0.this,arg1.this,arg2.this,arg3)
    val = TermStructurePtr(val)
    val.thisown = 1
    return val

def Stock(*args):
    val = apply(QuantLibc.Stock,()+args)
    val = InstrumentPtr(val)
    val.thisown = 1
    return val

def LinearInterpolation(arg0,arg1):
    val = QuantLibc.LinearInterpolation(arg0.this,arg1.this)
    val = InterpolationPtr(val)
    val.thisown = 1
    return val

def CubicSpline(arg0,arg1):
    val = QuantLibc.CubicSpline(arg0.this,arg1.this)
    val = InterpolationPtr(val)
    val.thisown = 1
    return val

Date_isLeap = QuantLibc.Date_isLeap

def Date_minDate():
    val = QuantLibc.Date_minDate()
    val = DatePtr(val)
    val.thisown = 1
    return val

def Date_maxDate():
    val = QuantLibc.Date_maxDate()
    val = DatePtr(val)
    val.thisown = 1
    return val



#-------------- VARIABLE WRAPPERS ------------------

