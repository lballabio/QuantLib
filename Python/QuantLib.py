# This file was created automatically by SWIG.
import QuantLibc
import new
class BoundaryCondition:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BoundaryCondition,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_BoundaryCondition(self)
    def __repr__(self):
        return "<C BoundaryCondition instance at %s>" % (self.this,)
class BoundaryConditionPtr(BoundaryCondition):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = BoundaryCondition


BoundaryCondition.type = new.instancemethod(QuantLibc.BoundaryCondition_type, None, BoundaryCondition)
BoundaryCondition.value = new.instancemethod(QuantLibc.BoundaryCondition_value, None, BoundaryCondition)

class Date:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Date,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Date(self)
    def plusDays(*args, **kwargs):
        val = apply(QuantLibc.Date_plusDays,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def plusWeeks(*args, **kwargs):
        val = apply(QuantLibc.Date_plusWeeks,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def plusMonths(*args, **kwargs):
        val = apply(QuantLibc.Date_plusMonths,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def plusYears(*args, **kwargs):
        val = apply(QuantLibc.Date_plusYears,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def plus(*args, **kwargs):
        val = apply(QuantLibc.Date_plus,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def __add__(*args, **kwargs):
        val = apply(QuantLibc.Date___add__,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def __sub__(*args, **kwargs):
        val = apply(QuantLibc.Date___sub__,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
class DatePtr(Date):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Date


Date.weekday = new.instancemethod(QuantLibc.Date_weekday, None, Date)
Date.dayOfMonth = new.instancemethod(QuantLibc.Date_dayOfMonth, None, Date)
Date.dayOfYear = new.instancemethod(QuantLibc.Date_dayOfYear, None, Date)
Date.month = new.instancemethod(QuantLibc.Date_month, None, Date)
Date.year = new.instancemethod(QuantLibc.Date_year, None, Date)
Date.serialNumber = new.instancemethod(QuantLibc.Date_serialNumber, None, Date)
Date.monthNumber = new.instancemethod(QuantLibc.Date_monthNumber, None, Date)
Date.weekdayNumber = new.instancemethod(QuantLibc.Date_weekdayNumber, None, Date)
Date.__cmp__ = new.instancemethod(QuantLibc.Date___cmp__, None, Date)
Date.__str__ = new.instancemethod(QuantLibc.Date___str__, None, Date)
Date.__repr__ = new.instancemethod(QuantLibc.Date___repr__, None, Date)

class Calendar:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Calendar(self)
    def roll(*args, **kwargs):
        val = apply(QuantLibc.Calendar_roll,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def advance(*args, **kwargs):
        val = apply(QuantLibc.Calendar_advance,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
class CalendarPtr(Calendar):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Calendar


Calendar.isBusinessDay = new.instancemethod(QuantLibc.Calendar_isBusinessDay, None, Calendar)
Calendar.isHoliday = new.instancemethod(QuantLibc.Calendar_isHoliday, None, Calendar)
Calendar.__str__ = new.instancemethod(QuantLibc.Calendar___str__, None, Calendar)
Calendar.__repr__ = new.instancemethod(QuantLibc.Calendar___repr__, None, Calendar)
Calendar.__cmp__ = new.instancemethod(QuantLibc.Calendar___cmp__, None, Calendar)
Calendar.__nonzero__ = new.instancemethod(QuantLibc.Calendar___nonzero__, None, Calendar)

class Currency:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Currency(self)
    def settlementCalendar(*args, **kwargs):
        val = apply(QuantLibc.Currency_settlementCalendar,args, kwargs)
        if val: val = CalendarPtr(val) ; val.thisown = 1
        return val
    def settlementDate(*args, **kwargs):
        val = apply(QuantLibc.Currency_settlementDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
class CurrencyPtr(Currency):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Currency


Currency.settlementDays = new.instancemethod(QuantLibc.Currency_settlementDays, None, Currency)
Currency.__str__ = new.instancemethod(QuantLibc.Currency___str__, None, Currency)
Currency.__repr__ = new.instancemethod(QuantLibc.Currency___repr__, None, Currency)
Currency.__cmp__ = new.instancemethod(QuantLibc.Currency___cmp__, None, Currency)
Currency.__nonzero__ = new.instancemethod(QuantLibc.Currency___nonzero__, None, Currency)

class DayCounter:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DayCounter(self)
class DayCounterPtr(DayCounter):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DayCounter


DayCounter.dayCount = new.instancemethod(QuantLibc.DayCounter_dayCount, None, DayCounter)
DayCounter.yearFraction = new.instancemethod(QuantLibc.DayCounter_yearFraction, None, DayCounter)
DayCounter.__str__ = new.instancemethod(QuantLibc.DayCounter___str__, None, DayCounter)
DayCounter.__repr__ = new.instancemethod(QuantLibc.DayCounter___repr__, None, DayCounter)
DayCounter.__cmp__ = new.instancemethod(QuantLibc.DayCounter___cmp__, None, DayCounter)
DayCounter.__nonzero__ = new.instancemethod(QuantLibc.DayCounter___nonzero__, None, DayCounter)

class NormalDistribution:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_NormalDistribution,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_NormalDistribution(self)
    def __repr__(self):
        return "<C NormalDistribution instance at %s>" % (self.this,)
class NormalDistributionPtr(NormalDistribution):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = NormalDistribution


NormalDistribution.derivative = new.instancemethod(QuantLibc.NormalDistribution_derivative, None, NormalDistribution)
NormalDistribution.__call__ = new.instancemethod(QuantLibc.NormalDistribution___call__, None, NormalDistribution)

class CumulativeNormalDistribution:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_CumulativeNormalDistribution,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_CumulativeNormalDistribution(self)
    def __repr__(self):
        return "<C CumulativeNormalDistribution instance at %s>" % (self.this,)
class CumulativeNormalDistributionPtr(CumulativeNormalDistribution):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = CumulativeNormalDistribution


CumulativeNormalDistribution.derivative = new.instancemethod(QuantLibc.CumulativeNormalDistribution_derivative, None, CumulativeNormalDistribution)
CumulativeNormalDistribution.__call__ = new.instancemethod(QuantLibc.CumulativeNormalDistribution___call__, None, CumulativeNormalDistribution)

class InvCumulativeNormalDistribution:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_InvCumulativeNormalDistribution,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_InvCumulativeNormalDistribution(self)
    def __repr__(self):
        return "<C InvCumulativeNormalDistribution instance at %s>" % (self.this,)
class InvCumulativeNormalDistributionPtr(InvCumulativeNormalDistribution):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = InvCumulativeNormalDistribution


InvCumulativeNormalDistribution.__call__ = new.instancemethod(QuantLibc.InvCumulativeNormalDistribution___call__, None, InvCumulativeNormalDistribution)

class Array:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Array,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Array(self)
    def __repr__(self):
        return "<C Array instance at %s>" % (self.this,)
class ArrayPtr(Array):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Array


Array.size = new.instancemethod(QuantLibc.Array_size, None, Array)
Array.__getitem__ = new.instancemethod(QuantLibc.Array___getitem__, None, Array)
Array.__setitem__ = new.instancemethod(QuantLibc.Array___setitem__, None, Array)
Array.__str__ = new.instancemethod(QuantLibc.Array___str__, None, Array)

class ArrayLexicographicalView:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_ArrayLexicographicalView(self)
    def __getitem__(*args, **kwargs):
        val = apply(QuantLibc.ArrayLexicographicalView___getitem__,args, kwargs)
        if val: val = ArrayLexicographicalViewColumnPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C ArrayLexicographicalView instance at %s>" % (self.this,)
class ArrayLexicographicalViewPtr(ArrayLexicographicalView):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = ArrayLexicographicalView


ArrayLexicographicalView.xSize = new.instancemethod(QuantLibc.ArrayLexicographicalView_xSize, None, ArrayLexicographicalView)
ArrayLexicographicalView.ySize = new.instancemethod(QuantLibc.ArrayLexicographicalView_ySize, None, ArrayLexicographicalView)
ArrayLexicographicalView.__str__ = new.instancemethod(QuantLibc.ArrayLexicographicalView___str__, None, ArrayLexicographicalView)

class ArrayLexicographicalViewColumn:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_ArrayLexicographicalViewColumn(self)
    def __repr__(self):
        return "<C ArrayLexicographicalViewColumn instance at %s>" % (self.this,)
class ArrayLexicographicalViewColumnPtr(ArrayLexicographicalViewColumn):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = ArrayLexicographicalViewColumn


ArrayLexicographicalViewColumn.__getitem__ = new.instancemethod(QuantLibc.ArrayLexicographicalViewColumn___getitem__, None, ArrayLexicographicalViewColumn)
ArrayLexicographicalViewColumn.__setitem__ = new.instancemethod(QuantLibc.ArrayLexicographicalViewColumn___setitem__, None, ArrayLexicographicalViewColumn)

class HistoryIterator:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryIterator(self)
    def date(*args, **kwargs):
        val = apply(QuantLibc.HistoryIterator_date,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C HistoryIterator instance at %s>" % (self.this,)
class HistoryIteratorPtr(HistoryIterator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = HistoryIterator


HistoryIterator.value = new.instancemethod(QuantLibc.HistoryIterator_value, None, HistoryIterator)
HistoryIterator.advance = new.instancemethod(QuantLibc.HistoryIterator_advance, None, HistoryIterator)
HistoryIterator.__cmp__ = new.instancemethod(QuantLibc.HistoryIterator___cmp__, None, HistoryIterator)
HistoryIterator.__str__ = new.instancemethod(QuantLibc.HistoryIterator___str__, None, HistoryIterator)

class HistoryValidIterator:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryValidIterator(self)
    def date(*args, **kwargs):
        val = apply(QuantLibc.HistoryValidIterator_date,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C HistoryValidIterator instance at %s>" % (self.this,)
class HistoryValidIteratorPtr(HistoryValidIterator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = HistoryValidIterator


HistoryValidIterator.value = new.instancemethod(QuantLibc.HistoryValidIterator_value, None, HistoryValidIterator)
HistoryValidIterator.advance = new.instancemethod(QuantLibc.HistoryValidIterator_advance, None, HistoryValidIterator)
HistoryValidIterator.__cmp__ = new.instancemethod(QuantLibc.HistoryValidIterator___cmp__, None, HistoryValidIterator)
HistoryValidIterator.__str__ = new.instancemethod(QuantLibc.HistoryValidIterator___str__, None, HistoryValidIterator)

class HistoryDataIterator:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryDataIterator(self)
    def __repr__(self):
        return "<C HistoryDataIterator instance at %s>" % (self.this,)
class HistoryDataIteratorPtr(HistoryDataIterator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = HistoryDataIterator


HistoryDataIterator.__float__ = new.instancemethod(QuantLibc.HistoryDataIterator___float__, None, HistoryDataIterator)
HistoryDataIterator.advance = new.instancemethod(QuantLibc.HistoryDataIterator_advance, None, HistoryDataIterator)
HistoryDataIterator.__cmp__ = new.instancemethod(QuantLibc.HistoryDataIterator___cmp__, None, HistoryDataIterator)
HistoryDataIterator.__str__ = new.instancemethod(QuantLibc.HistoryDataIterator___str__, None, HistoryDataIterator)

class HistoryValidDataIterator:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_HistoryValidDataIterator(self)
    def __repr__(self):
        return "<C HistoryValidDataIterator instance at %s>" % (self.this,)
class HistoryValidDataIteratorPtr(HistoryValidDataIterator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = HistoryValidDataIterator


HistoryValidDataIterator.__float__ = new.instancemethod(QuantLibc.HistoryValidDataIterator___float__, None, HistoryValidDataIterator)
HistoryValidDataIterator.advance = new.instancemethod(QuantLibc.HistoryValidDataIterator_advance, None, HistoryValidDataIterator)
HistoryValidDataIterator.__cmp__ = new.instancemethod(QuantLibc.HistoryValidDataIterator___cmp__, None, HistoryValidDataIterator)
HistoryValidDataIterator.__str__ = new.instancemethod(QuantLibc.HistoryValidDataIterator___str__, None, HistoryValidDataIterator)

class History:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_History,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_History(self)
    def firstDate(*args, **kwargs):
        val = apply(QuantLibc.History_firstDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def lastDate(*args, **kwargs):
        val = apply(QuantLibc.History_lastDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def begin(*args, **kwargs):
        val = apply(QuantLibc.History_begin,args, kwargs)
        if val: val = HistoryIteratorPtr(val) ; val.thisown = 1
        return val
    def end(*args, **kwargs):
        val = apply(QuantLibc.History_end,args, kwargs)
        if val: val = HistoryIteratorPtr(val) ; val.thisown = 1
        return val
    def iterator(*args, **kwargs):
        val = apply(QuantLibc.History_iterator,args, kwargs)
        if val: val = HistoryIteratorPtr(val) ; val.thisown = 1
        return val
    def vbegin(*args, **kwargs):
        val = apply(QuantLibc.History_vbegin,args, kwargs)
        if val: val = HistoryValidIteratorPtr(val) ; val.thisown = 1
        return val
    def vend(*args, **kwargs):
        val = apply(QuantLibc.History_vend,args, kwargs)
        if val: val = HistoryValidIteratorPtr(val) ; val.thisown = 1
        return val
    def valid_iterator(*args, **kwargs):
        val = apply(QuantLibc.History_valid_iterator,args, kwargs)
        if val: val = HistoryValidIteratorPtr(val) ; val.thisown = 1
        return val
    def dbegin(*args, **kwargs):
        val = apply(QuantLibc.History_dbegin,args, kwargs)
        if val: val = HistoryDataIteratorPtr(val) ; val.thisown = 1
        return val
    def dend(*args, **kwargs):
        val = apply(QuantLibc.History_dend,args, kwargs)
        if val: val = HistoryDataIteratorPtr(val) ; val.thisown = 1
        return val
    def data_iterator(*args, **kwargs):
        val = apply(QuantLibc.History_data_iterator,args, kwargs)
        if val: val = HistoryDataIteratorPtr(val) ; val.thisown = 1
        return val
    def vdbegin(*args, **kwargs):
        val = apply(QuantLibc.History_vdbegin,args, kwargs)
        if val: val = HistoryValidDataIteratorPtr(val) ; val.thisown = 1
        return val
    def vdend(*args, **kwargs):
        val = apply(QuantLibc.History_vdend,args, kwargs)
        if val: val = HistoryValidDataIteratorPtr(val) ; val.thisown = 1
        return val
    def valid_data_iterator(*args, **kwargs):
        val = apply(QuantLibc.History_valid_data_iterator,args, kwargs)
        if val: val = HistoryValidDataIteratorPtr(val) ; val.thisown = 1
        return val
class HistoryPtr(History):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = History


History.size = new.instancemethod(QuantLibc.History_size, None, History)
History.__getitem__ = new.instancemethod(QuantLibc.History___getitem__, None, History)
History.__str__ = new.instancemethod(QuantLibc.History___str__, None, History)
History.__repr__ = new.instancemethod(QuantLibc.History___repr__, None, History)

class TermStructure:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_TermStructure(self)
    def clone(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_clone,args, kwargs)
        if val: val = TermStructurePtr(val) ; val.thisown = 1
        return val
    def currency(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_currency,args, kwargs)
        if val: val = CurrencyPtr(val) ; val.thisown = 1
        return val
    def todaysDate(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_todaysDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def settlementDate(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_settlementDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def calendar(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_calendar,args, kwargs)
        if val: val = CalendarPtr(val) ; val.thisown = 1
        return val
    def maxDate(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_maxDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def minDate(*args, **kwargs):
        val = apply(QuantLibc.TermStructure_minDate,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C TermStructure instance at %s>" % (self.this,)
class TermStructurePtr(TermStructure):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = TermStructure


TermStructure.zeroYield = new.instancemethod(QuantLibc.TermStructure_zeroYield, None, TermStructure)
TermStructure.discount = new.instancemethod(QuantLibc.TermStructure_discount, None, TermStructure)
TermStructure.forward = new.instancemethod(QuantLibc.TermStructure_forward, None, TermStructure)
TermStructure.__nonzero__ = new.instancemethod(QuantLibc.TermStructure___nonzero__, None, TermStructure)

class DepositRate:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DepositRate,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DepositRate(self)
    def maturity(*args, **kwargs):
        val = apply(QuantLibc.DepositRate_maturity,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def dayCounter(*args, **kwargs):
        val = apply(QuantLibc.DepositRate_dayCounter,args, kwargs)
        if val: val = DayCounterPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C DepositRate instance at %s>" % (self.this,)
class DepositRatePtr(DepositRate):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DepositRate


DepositRate.rate = new.instancemethod(QuantLibc.DepositRate_rate, None, DepositRate)

class Instrument:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Instrument(self)
    def termStructure(*args, **kwargs):
        val = apply(QuantLibc.Instrument_termStructure,args, kwargs)
        if val: val = TermStructurePtr(val) ; val.thisown = 1
        return val
class InstrumentPtr(Instrument):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Instrument


Instrument.setPrice = new.instancemethod(QuantLibc.Instrument_setPrice, None, Instrument)
Instrument.setTermStructure = new.instancemethod(QuantLibc.Instrument_setTermStructure, None, Instrument)
Instrument.isinCode = new.instancemethod(QuantLibc.Instrument_isinCode, None, Instrument)
Instrument.description = new.instancemethod(QuantLibc.Instrument_description, None, Instrument)
Instrument.NPV = new.instancemethod(QuantLibc.Instrument_NPV, None, Instrument)
Instrument.price = new.instancemethod(QuantLibc.Instrument_price, None, Instrument)
Instrument.__str__ = new.instancemethod(QuantLibc.Instrument___str__, None, Instrument)
Instrument.__repr__ = new.instancemethod(QuantLibc.Instrument___repr__, None, Instrument)
Instrument.__cmp__ = new.instancemethod(QuantLibc.Instrument___cmp__, None, Instrument)
Instrument.__nonzero__ = new.instancemethod(QuantLibc.Instrument___nonzero__, None, Instrument)

class Interpolation:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Interpolation(self)
    def __repr__(self):
        return "<C Interpolation instance at %s>" % (self.this,)
class InterpolationPtr(Interpolation):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Interpolation


Interpolation.__call__ = new.instancemethod(QuantLibc.Interpolation___call__, None, Interpolation)
Interpolation.__nonzero__ = new.instancemethod(QuantLibc.Interpolation___nonzero__, None, Interpolation)

class StandardPathGenerator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_StandardPathGenerator,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C StandardPathGenerator instance at %s>" % (self.this,)
class StandardPathGeneratorPtr(StandardPathGenerator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = StandardPathGenerator


StandardPathGenerator.next = new.instancemethod(QuantLibc.StandardPathGenerator_next, None, StandardPathGenerator)
StandardPathGenerator.weight = new.instancemethod(QuantLibc.StandardPathGenerator_weight, None, StandardPathGenerator)

class McEuropeanPricer:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_McEuropeanPricer,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C McEuropeanPricer instance at %s>" % (self.this,)
class McEuropeanPricerPtr(McEuropeanPricer):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = McEuropeanPricer


McEuropeanPricer.value = new.instancemethod(QuantLibc.McEuropeanPricer_value, None, McEuropeanPricer)
McEuropeanPricer.errorEstimate = new.instancemethod(QuantLibc.McEuropeanPricer_errorEstimate, None, McEuropeanPricer)

class GeometricAsianOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_GeometricAsianOption,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C GeometricAsianOption instance at %s>" % (self.this,)
class GeometricAsianOptionPtr(GeometricAsianOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = GeometricAsianOption


GeometricAsianOption.value = new.instancemethod(QuantLibc.GeometricAsianOption_value, None, GeometricAsianOption)

class McAsianPricer:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_McAsianPricer,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C McAsianPricer instance at %s>" % (self.this,)
class McAsianPricerPtr(McAsianPricer):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = McAsianPricer


McAsianPricer.value = new.instancemethod(QuantLibc.McAsianPricer_value, None, McAsianPricer)
McAsianPricer.errorEstimate = new.instancemethod(QuantLibc.McAsianPricer_errorEstimate, None, McAsianPricer)

class TridiagonalOperator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_TridiagonalOperator,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_TridiagonalOperator(self)
    def __repr__(self):
        return "<C TridiagonalOperator instance at %s>" % (self.this,)
class TridiagonalOperatorPtr(TridiagonalOperator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = TridiagonalOperator


TridiagonalOperator.solveFor = new.instancemethod(QuantLibc.TridiagonalOperator_solveFor, None, TridiagonalOperator)
TridiagonalOperator.applyTo = new.instancemethod(QuantLibc.TridiagonalOperator_applyTo, None, TridiagonalOperator)
TridiagonalOperator.size = new.instancemethod(QuantLibc.TridiagonalOperator_size, None, TridiagonalOperator)
TridiagonalOperator.setLowerBC = new.instancemethod(QuantLibc.TridiagonalOperator_setLowerBC, None, TridiagonalOperator)
TridiagonalOperator.setHigherBC = new.instancemethod(QuantLibc.TridiagonalOperator_setHigherBC, None, TridiagonalOperator)
TridiagonalOperator.setFirstRow = new.instancemethod(QuantLibc.TridiagonalOperator_setFirstRow, None, TridiagonalOperator)
TridiagonalOperator.setMidRow = new.instancemethod(QuantLibc.TridiagonalOperator_setMidRow, None, TridiagonalOperator)
TridiagonalOperator.setMidRows = new.instancemethod(QuantLibc.TridiagonalOperator_setMidRows, None, TridiagonalOperator)
TridiagonalOperator.setLastRow = new.instancemethod(QuantLibc.TridiagonalOperator_setLastRow, None, TridiagonalOperator)

class DPlus(TridiagonalOperator):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DPlus,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DPlus(self)
    def __repr__(self):
        return "<C DPlus instance at %s>" % (self.this,)
class DPlusPtr(DPlus):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DPlus



class DMinus(TridiagonalOperator):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DMinus,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DMinus(self)
    def __repr__(self):
        return "<C DMinus instance at %s>" % (self.this,)
class DMinusPtr(DMinus):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DMinus



class DZero(TridiagonalOperator):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DZero,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DZero(self)
    def __repr__(self):
        return "<C DZero instance at %s>" % (self.this,)
class DZeroPtr(DZero):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DZero



class DPlusDMinus(TridiagonalOperator):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DPlusDMinus,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DPlusDMinus(self)
    def __repr__(self):
        return "<C DPlusDMinus instance at %s>" % (self.this,)
class DPlusDMinusPtr(DPlusDMinus):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DPlusDMinus



class BSMEuropeanOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BSMEuropeanOption,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_BSMEuropeanOption(self)
    def __repr__(self):
        return "<C BSMEuropeanOption instance at %s>" % (self.this,)
class BSMEuropeanOptionPtr(BSMEuropeanOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = BSMEuropeanOption


BSMEuropeanOption.setVolatility = new.instancemethod(QuantLibc.BSMEuropeanOption_setVolatility, None, BSMEuropeanOption)
BSMEuropeanOption.setRiskFreeRate = new.instancemethod(QuantLibc.BSMEuropeanOption_setRiskFreeRate, None, BSMEuropeanOption)
BSMEuropeanOption.value = new.instancemethod(QuantLibc.BSMEuropeanOption_value, None, BSMEuropeanOption)
BSMEuropeanOption.delta = new.instancemethod(QuantLibc.BSMEuropeanOption_delta, None, BSMEuropeanOption)
BSMEuropeanOption.gamma = new.instancemethod(QuantLibc.BSMEuropeanOption_gamma, None, BSMEuropeanOption)
BSMEuropeanOption.theta = new.instancemethod(QuantLibc.BSMEuropeanOption_theta, None, BSMEuropeanOption)
BSMEuropeanOption.vega = new.instancemethod(QuantLibc.BSMEuropeanOption_vega, None, BSMEuropeanOption)
BSMEuropeanOption.rho = new.instancemethod(QuantLibc.BSMEuropeanOption_rho, None, BSMEuropeanOption)
BSMEuropeanOption.impliedVolatility = new.instancemethod(QuantLibc.BSMEuropeanOption_impliedVolatility, None, BSMEuropeanOption)

class BSMAmericanOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BSMAmericanOption,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_BSMAmericanOption(self)
    def __repr__(self):
        return "<C BSMAmericanOption instance at %s>" % (self.this,)
class BSMAmericanOptionPtr(BSMAmericanOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = BSMAmericanOption


BSMAmericanOption.setVolatility = new.instancemethod(QuantLibc.BSMAmericanOption_setVolatility, None, BSMAmericanOption)
BSMAmericanOption.setRiskFreeRate = new.instancemethod(QuantLibc.BSMAmericanOption_setRiskFreeRate, None, BSMAmericanOption)
BSMAmericanOption.value = new.instancemethod(QuantLibc.BSMAmericanOption_value, None, BSMAmericanOption)
BSMAmericanOption.delta = new.instancemethod(QuantLibc.BSMAmericanOption_delta, None, BSMAmericanOption)
BSMAmericanOption.gamma = new.instancemethod(QuantLibc.BSMAmericanOption_gamma, None, BSMAmericanOption)
BSMAmericanOption.theta = new.instancemethod(QuantLibc.BSMAmericanOption_theta, None, BSMAmericanOption)
BSMAmericanOption.vega = new.instancemethod(QuantLibc.BSMAmericanOption_vega, None, BSMAmericanOption)
BSMAmericanOption.rho = new.instancemethod(QuantLibc.BSMAmericanOption_rho, None, BSMAmericanOption)
BSMAmericanOption.impliedVolatility = new.instancemethod(QuantLibc.BSMAmericanOption_impliedVolatility, None, BSMAmericanOption)

class DividendAmericanOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DividendAmericanOption,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DividendAmericanOption(self)
    def __repr__(self):
        return "<C DividendAmericanOption instance at %s>" % (self.this,)
class DividendAmericanOptionPtr(DividendAmericanOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DividendAmericanOption


DividendAmericanOption.value = new.instancemethod(QuantLibc.DividendAmericanOption_value, None, DividendAmericanOption)
DividendAmericanOption.delta = new.instancemethod(QuantLibc.DividendAmericanOption_delta, None, DividendAmericanOption)
DividendAmericanOption.gamma = new.instancemethod(QuantLibc.DividendAmericanOption_gamma, None, DividendAmericanOption)
DividendAmericanOption.theta = new.instancemethod(QuantLibc.DividendAmericanOption_theta, None, DividendAmericanOption)
DividendAmericanOption.vega = new.instancemethod(QuantLibc.DividendAmericanOption_vega, None, DividendAmericanOption)
DividendAmericanOption.rho = new.instancemethod(QuantLibc.DividendAmericanOption_rho, None, DividendAmericanOption)
DividendAmericanOption.impliedVolatility = new.instancemethod(QuantLibc.DividendAmericanOption_impliedVolatility, None, DividendAmericanOption)

class DividendEuropeanOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DividendEuropeanOption,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DividendEuropeanOption(self)
    def __repr__(self):
        return "<C DividendEuropeanOption instance at %s>" % (self.this,)
class DividendEuropeanOptionPtr(DividendEuropeanOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DividendEuropeanOption


DividendEuropeanOption.value = new.instancemethod(QuantLibc.DividendEuropeanOption_value, None, DividendEuropeanOption)
DividendEuropeanOption.delta = new.instancemethod(QuantLibc.DividendEuropeanOption_delta, None, DividendEuropeanOption)
DividendEuropeanOption.gamma = new.instancemethod(QuantLibc.DividendEuropeanOption_gamma, None, DividendEuropeanOption)
DividendEuropeanOption.theta = new.instancemethod(QuantLibc.DividendEuropeanOption_theta, None, DividendEuropeanOption)
DividendEuropeanOption.vega = new.instancemethod(QuantLibc.DividendEuropeanOption_vega, None, DividendEuropeanOption)
DividendEuropeanOption.rho = new.instancemethod(QuantLibc.DividendEuropeanOption_rho, None, DividendEuropeanOption)
DividendEuropeanOption.impliedVolatility = new.instancemethod(QuantLibc.DividendEuropeanOption_impliedVolatility, None, DividendEuropeanOption)

class UniformRandomGenerator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_UniformRandomGenerator,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_UniformRandomGenerator(self)
    def __repr__(self):
        return "<C UniformRandomGenerator instance at %s>" % (self.this,)
class UniformRandomGeneratorPtr(UniformRandomGenerator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = UniformRandomGenerator


UniformRandomGenerator.next = new.instancemethod(QuantLibc.UniformRandomGenerator_next, None, UniformRandomGenerator)
UniformRandomGenerator.weight = new.instancemethod(QuantLibc.UniformRandomGenerator_weight, None, UniformRandomGenerator)

class GaussianRandomGenerator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_GaussianRandomGenerator,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_GaussianRandomGenerator(self)
    def __repr__(self):
        return "<C GaussianRandomGenerator instance at %s>" % (self.this,)
class GaussianRandomGeneratorPtr(GaussianRandomGenerator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = GaussianRandomGenerator


GaussianRandomGenerator.next = new.instancemethod(QuantLibc.GaussianRandomGenerator_next, None, GaussianRandomGenerator)
GaussianRandomGenerator.weight = new.instancemethod(QuantLibc.GaussianRandomGenerator_weight, None, GaussianRandomGenerator)

class RiskStatistics:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_RiskStatistics,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_RiskStatistics(self)
    def __repr__(self):
        return "<C RiskStatistics instance at %s>" % (self.this,)
class RiskStatisticsPtr(RiskStatistics):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = RiskStatistics


RiskStatistics.samples = new.instancemethod(QuantLibc.RiskStatistics_samples, None, RiskStatistics)
RiskStatistics.weightSum = new.instancemethod(QuantLibc.RiskStatistics_weightSum, None, RiskStatistics)
RiskStatistics.mean = new.instancemethod(QuantLibc.RiskStatistics_mean, None, RiskStatistics)
RiskStatistics.variance = new.instancemethod(QuantLibc.RiskStatistics_variance, None, RiskStatistics)
RiskStatistics.standardDeviation = new.instancemethod(QuantLibc.RiskStatistics_standardDeviation, None, RiskStatistics)
RiskStatistics.errorEstimate = new.instancemethod(QuantLibc.RiskStatistics_errorEstimate, None, RiskStatistics)
RiskStatistics.skewness = new.instancemethod(QuantLibc.RiskStatistics_skewness, None, RiskStatistics)
RiskStatistics.kurtosis = new.instancemethod(QuantLibc.RiskStatistics_kurtosis, None, RiskStatistics)
RiskStatistics.min = new.instancemethod(QuantLibc.RiskStatistics_min, None, RiskStatistics)
RiskStatistics.max = new.instancemethod(QuantLibc.RiskStatistics_max, None, RiskStatistics)
RiskStatistics.valueAtRisk = new.instancemethod(QuantLibc.RiskStatistics_valueAtRisk, None, RiskStatistics)
RiskStatistics.shortfall = new.instancemethod(QuantLibc.RiskStatistics_shortfall, None, RiskStatistics)
RiskStatistics.averageShortfall = new.instancemethod(QuantLibc.RiskStatistics_averageShortfall, None, RiskStatistics)
RiskStatistics.add = new.instancemethod(QuantLibc.RiskStatistics_add, None, RiskStatistics)
RiskStatistics.reset = new.instancemethod(QuantLibc.RiskStatistics_reset, None, RiskStatistics)
RiskStatistics.addSequence = new.instancemethod(QuantLibc.RiskStatistics_addSequence, None, RiskStatistics)
RiskStatistics.addWeightedSequence = new.instancemethod(QuantLibc.RiskStatistics_addWeightedSequence, None, RiskStatistics)

class ObjectiveFunction:
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C ObjectiveFunction instance at %s>" % (self.this,)
class ObjectiveFunctionPtr(ObjectiveFunction):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = ObjectiveFunction


ObjectiveFunction.value = new.instancemethod(QuantLibc.ObjectiveFunction_value, None, ObjectiveFunction)
ObjectiveFunction.derivative = new.instancemethod(QuantLibc.ObjectiveFunction_derivative, None, ObjectiveFunction)

class Solver1D:
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C Solver1D instance at %s>" % (self.this,)
class Solver1DPtr(Solver1D):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Solver1D


Solver1D.solve = new.instancemethod(QuantLibc.Solver1D_solve, None, Solver1D)
Solver1D.bracketedSolve = new.instancemethod(QuantLibc.Solver1D_bracketedSolve, None, Solver1D)
Solver1D.setMaxEvaluations = new.instancemethod(QuantLibc.Solver1D_setMaxEvaluations, None, Solver1D)
Solver1D.pySolve = new.instancemethod(QuantLibc.Solver1D_pySolve, None, Solver1D)
Solver1D.pyBracketedSolve = new.instancemethod(QuantLibc.Solver1D_pyBracketedSolve, None, Solver1D)

class Bisection(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Bisection,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Bisection(self)
    def __repr__(self):
        return "<C Bisection instance at %s>" % (self.this,)
class BisectionPtr(Bisection):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Bisection



class Brent(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Brent,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Brent(self)
    def __repr__(self):
        return "<C Brent instance at %s>" % (self.this,)
class BrentPtr(Brent):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Brent



class FalsePosition(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_FalsePosition,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_FalsePosition(self)
    def __repr__(self):
        return "<C FalsePosition instance at %s>" % (self.this,)
class FalsePositionPtr(FalsePosition):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = FalsePosition



class Newton(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Newton,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Newton(self)
    def __repr__(self):
        return "<C Newton instance at %s>" % (self.this,)
class NewtonPtr(Newton):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Newton



class NewtonSafe(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_NewtonSafe,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_NewtonSafe(self)
    def __repr__(self):
        return "<C NewtonSafe instance at %s>" % (self.this,)
class NewtonSafePtr(NewtonSafe):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = NewtonSafe



class Ridder(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Ridder,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Ridder(self)
    def __repr__(self):
        return "<C Ridder instance at %s>" % (self.this,)
class RidderPtr(Ridder):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Ridder



class Secant(Solver1D):
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Secant,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Secant(self)
    def __repr__(self):
        return "<C Secant instance at %s>" % (self.this,)
class SecantPtr(Secant):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Secant



class Statistics:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Statistics,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Statistics(self)
    def __repr__(self):
        return "<C Statistics instance at %s>" % (self.this,)
class StatisticsPtr(Statistics):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Statistics


Statistics.samples = new.instancemethod(QuantLibc.Statistics_samples, None, Statistics)
Statistics.weightSum = new.instancemethod(QuantLibc.Statistics_weightSum, None, Statistics)
Statistics.mean = new.instancemethod(QuantLibc.Statistics_mean, None, Statistics)
Statistics.variance = new.instancemethod(QuantLibc.Statistics_variance, None, Statistics)
Statistics.standardDeviation = new.instancemethod(QuantLibc.Statistics_standardDeviation, None, Statistics)
Statistics.errorEstimate = new.instancemethod(QuantLibc.Statistics_errorEstimate, None, Statistics)
Statistics.skewness = new.instancemethod(QuantLibc.Statistics_skewness, None, Statistics)
Statistics.kurtosis = new.instancemethod(QuantLibc.Statistics_kurtosis, None, Statistics)
Statistics.min = new.instancemethod(QuantLibc.Statistics_min, None, Statistics)
Statistics.max = new.instancemethod(QuantLibc.Statistics_max, None, Statistics)
Statistics.add = new.instancemethod(QuantLibc.Statistics_add, None, Statistics)
Statistics.reset = new.instancemethod(QuantLibc.Statistics_reset, None, Statistics)
Statistics.addSequence = new.instancemethod(QuantLibc.Statistics_addSequence, None, Statistics)
Statistics.addWeightedSequence = new.instancemethod(QuantLibc.Statistics_addWeightedSequence, None, Statistics)



#-------------- FUNCTION WRAPPERS ------------------

def DateFromSerialNumber(*args, **kwargs):
    val = apply(QuantLibc.DateFromSerialNumber,args,kwargs)
    if val: val = DatePtr(val); val.thisown = 1
    return val

def TARGET(*args, **kwargs):
    val = apply(QuantLibc.TARGET,args,kwargs)
    if val: val = CalendarPtr(val); val.thisown = 1
    return val

def NewYork(*args, **kwargs):
    val = apply(QuantLibc.NewYork,args,kwargs)
    if val: val = CalendarPtr(val); val.thisown = 1
    return val

def London(*args, **kwargs):
    val = apply(QuantLibc.London,args,kwargs)
    if val: val = CalendarPtr(val); val.thisown = 1
    return val

def Frankfurt(*args, **kwargs):
    val = apply(QuantLibc.Frankfurt,args,kwargs)
    if val: val = CalendarPtr(val); val.thisown = 1
    return val

def Milan(*args, **kwargs):
    val = apply(QuantLibc.Milan,args,kwargs)
    if val: val = CalendarPtr(val); val.thisown = 1
    return val

def Zurich(*args, **kwargs):
    val = apply(QuantLibc.Zurich,args,kwargs)
    if val: val = CalendarPtr(val); val.thisown = 1
    return val

def EUR(*args, **kwargs):
    val = apply(QuantLibc.EUR,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def USD(*args, **kwargs):
    val = apply(QuantLibc.USD,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def GBP(*args, **kwargs):
    val = apply(QuantLibc.GBP,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def DEM(*args, **kwargs):
    val = apply(QuantLibc.DEM,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def ITL(*args, **kwargs):
    val = apply(QuantLibc.ITL,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def AUD(*args, **kwargs):
    val = apply(QuantLibc.AUD,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def CAD(*args, **kwargs):
    val = apply(QuantLibc.CAD,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def CHF(*args, **kwargs):
    val = apply(QuantLibc.CHF,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def DKK(*args, **kwargs):
    val = apply(QuantLibc.DKK,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def JPY(*args, **kwargs):
    val = apply(QuantLibc.JPY,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def SEK(*args, **kwargs):
    val = apply(QuantLibc.SEK,args,kwargs)
    if val: val = CurrencyPtr(val); val.thisown = 1
    return val

def Actual360(*args, **kwargs):
    val = apply(QuantLibc.Actual360,args,kwargs)
    if val: val = DayCounterPtr(val); val.thisown = 1
    return val

def Actual365(*args, **kwargs):
    val = apply(QuantLibc.Actual365,args,kwargs)
    if val: val = DayCounterPtr(val); val.thisown = 1
    return val

def Thirty360(*args, **kwargs):
    val = apply(QuantLibc.Thirty360,args,kwargs)
    if val: val = DayCounterPtr(val); val.thisown = 1
    return val

def Thirty360European(*args, **kwargs):
    val = apply(QuantLibc.Thirty360European,args,kwargs)
    if val: val = DayCounterPtr(val); val.thisown = 1
    return val

def Thirty360Italian(*args, **kwargs):
    val = apply(QuantLibc.Thirty360Italian,args,kwargs)
    if val: val = DayCounterPtr(val); val.thisown = 1
    return val

def LexicographicalView(*args, **kwargs):
    val = apply(QuantLibc.LexicographicalView,args,kwargs)
    if val: val = ArrayLexicographicalViewPtr(val); val.thisown = 1
    return val

def ImpliedTermStructure(*args, **kwargs):
    val = apply(QuantLibc.ImpliedTermStructure,args,kwargs)
    if val: val = TermStructurePtr(val); val.thisown = 1
    return val

def SpreadedTermStructure(*args, **kwargs):
    val = apply(QuantLibc.SpreadedTermStructure,args,kwargs)
    if val: val = TermStructurePtr(val); val.thisown = 1
    return val

def FlatForward(*args, **kwargs):
    val = apply(QuantLibc.FlatForward,args,kwargs)
    if val: val = TermStructurePtr(val); val.thisown = 1
    return val

def PiecewiseConstantForwards(*args, **kwargs):
    val = apply(QuantLibc.PiecewiseConstantForwards,args,kwargs)
    if val: val = TermStructurePtr(val); val.thisown = 1
    return val

def Stock(*args, **kwargs):
    val = apply(QuantLibc.Stock,args,kwargs)
    if val: val = InstrumentPtr(val); val.thisown = 1
    return val

def LinearInterpolation(*args, **kwargs):
    val = apply(QuantLibc.LinearInterpolation,args,kwargs)
    if val: val = InterpolationPtr(val); val.thisown = 1
    return val

def CubicSpline(*args, **kwargs):
    val = apply(QuantLibc.CubicSpline,args,kwargs)
    if val: val = InterpolationPtr(val); val.thisown = 1
    return val

Date_isLeap = QuantLibc.Date_isLeap

def Date_minDate(*args, **kwargs):
    val = apply(QuantLibc.Date_minDate,args,kwargs)
    if val: val = DatePtr(val); val.thisown = 1
    return val

def Date_maxDate(*args, **kwargs):
    val = apply(QuantLibc.Date_maxDate,args,kwargs)
    if val: val = DatePtr(val); val.thisown = 1
    return val



#-------------- VARIABLE WRAPPERS ------------------

