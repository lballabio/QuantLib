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
Date.__nonzero__ = new.instancemethod(QuantLibc.Date___nonzero__, None, Date)

class DateVector:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DateVector(self)
    def __getitem__(*args, **kwargs):
        val = apply(QuantLibc.DateVector___getitem__,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def __getslice__(*args, **kwargs):
        val = apply(QuantLibc.DateVector___getslice__,args, kwargs)
        if val: val = DateVectorPtr(val) ; val.thisown = 1
        return val
class DateVectorPtr(DateVector):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DateVector


DateVector.__len__ = new.instancemethod(QuantLibc.DateVector___len__, None, DateVector)
DateVector.__setitem__ = new.instancemethod(QuantLibc.DateVector___setitem__, None, DateVector)
DateVector.__setslice__ = new.instancemethod(QuantLibc.DateVector___setslice__, None, DateVector)
DateVector.__str__ = new.instancemethod(QuantLibc.DateVector___str__, None, DateVector)
DateVector.__repr__ = new.instancemethod(QuantLibc.DateVector___repr__, None, DateVector)
DateVector.__nonzero__ = new.instancemethod(QuantLibc.DateVector___nonzero__, None, DateVector)

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

class IntVector:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_IntVector(self)
    def __getslice__(*args, **kwargs):
        val = apply(QuantLibc.IntVector___getslice__,args, kwargs)
        if val: val = IntVectorPtr(val) ; val.thisown = 1
        return val
class IntVectorPtr(IntVector):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = IntVector


IntVector.__len__ = new.instancemethod(QuantLibc.IntVector___len__, None, IntVector)
IntVector.__getitem__ = new.instancemethod(QuantLibc.IntVector___getitem__, None, IntVector)
IntVector.__setitem__ = new.instancemethod(QuantLibc.IntVector___setitem__, None, IntVector)
IntVector.__setslice__ = new.instancemethod(QuantLibc.IntVector___setslice__, None, IntVector)
IntVector.__str__ = new.instancemethod(QuantLibc.IntVector___str__, None, IntVector)
IntVector.__repr__ = new.instancemethod(QuantLibc.IntVector___repr__, None, IntVector)
IntVector.__nonzero__ = new.instancemethod(QuantLibc.IntVector___nonzero__, None, IntVector)

class DoubleVector:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_DoubleVector(self)
    def __getslice__(*args, **kwargs):
        val = apply(QuantLibc.DoubleVector___getslice__,args, kwargs)
        if val: val = DoubleVectorPtr(val) ; val.thisown = 1
        return val
class DoubleVectorPtr(DoubleVector):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = DoubleVector


DoubleVector.__len__ = new.instancemethod(QuantLibc.DoubleVector___len__, None, DoubleVector)
DoubleVector.__getitem__ = new.instancemethod(QuantLibc.DoubleVector___getitem__, None, DoubleVector)
DoubleVector.__setitem__ = new.instancemethod(QuantLibc.DoubleVector___setitem__, None, DoubleVector)
DoubleVector.__setslice__ = new.instancemethod(QuantLibc.DoubleVector___setslice__, None, DoubleVector)
DoubleVector.__str__ = new.instancemethod(QuantLibc.DoubleVector___str__, None, DoubleVector)
DoubleVector.__repr__ = new.instancemethod(QuantLibc.DoubleVector___repr__, None, DoubleVector)
DoubleVector.__nonzero__ = new.instancemethod(QuantLibc.DoubleVector___nonzero__, None, DoubleVector)

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

class Array:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Array(self)
    def __getslice__(*args, **kwargs):
        val = apply(QuantLibc.Array___getslice__,args, kwargs)
        if val: val = ArrayPtr(val) ; val.thisown = 1
        return val
class ArrayPtr(Array):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Array


Array.__len__ = new.instancemethod(QuantLibc.Array___len__, None, Array)
Array.__getitem__ = new.instancemethod(QuantLibc.Array___getitem__, None, Array)
Array.__setitem__ = new.instancemethod(QuantLibc.Array___setitem__, None, Array)
Array.__setslice__ = new.instancemethod(QuantLibc.Array___setslice__, None, Array)
Array.__str__ = new.instancemethod(QuantLibc.Array___str__, None, Array)
Array.__repr__ = new.instancemethod(QuantLibc.Array___repr__, None, Array)
Array.__nonzero__ = new.instancemethod(QuantLibc.Array___nonzero__, None, Array)

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

class Matrix:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Matrix,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Matrix(self)
    def __getitem__(*args, **kwargs):
        val = apply(QuantLibc.Matrix___getitem__,args, kwargs)
        if val: val = MatrixRowPtr(val) ; val.thisown = 1
        return val
    def __add__(*args, **kwargs):
        val = apply(QuantLibc.Matrix___add__,args, kwargs)
        if val: val = MatrixPtr(val) ; val.thisown = 1
        return val
    def __sub__(*args, **kwargs):
        val = apply(QuantLibc.Matrix___sub__,args, kwargs)
        if val: val = MatrixPtr(val) ; val.thisown = 1
        return val
    def __mul__(*args, **kwargs):
        val = apply(QuantLibc.Matrix___mul__,args, kwargs)
        if val: val = MatrixPtr(val) ; val.thisown = 1
        return val
    def __div__(*args, **kwargs):
        val = apply(QuantLibc.Matrix___div__,args, kwargs)
        if val: val = MatrixPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C Matrix instance at %s>" % (self.this,)
class MatrixPtr(Matrix):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Matrix


Matrix.rows = new.instancemethod(QuantLibc.Matrix_rows, None, Matrix)
Matrix.columns = new.instancemethod(QuantLibc.Matrix_columns, None, Matrix)
Matrix.__str__ = new.instancemethod(QuantLibc.Matrix___str__, None, Matrix)

class MatrixRow:
    def __init__(self,this):
        self.this = this

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_MatrixRow(self)
    def __repr__(self):
        return "<C MatrixRow instance at %s>" % (self.this,)
class MatrixRowPtr(MatrixRow):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = MatrixRow


MatrixRow.__getitem__ = new.instancemethod(QuantLibc.MatrixRow___getitem__, None, MatrixRow)
MatrixRow.__setitem__ = new.instancemethod(QuantLibc.MatrixRow___setitem__, None, MatrixRow)

class StandardPathGenerator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_StandardPathGenerator,args,kwargs)
        self.thisown = 1

    def next(*args, **kwargs):
        val = apply(QuantLibc.StandardPathGenerator_next,args, kwargs)
        if val: val = ArrayPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C StandardPathGenerator instance at %s>" % (self.this,)
class StandardPathGeneratorPtr(StandardPathGenerator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = StandardPathGenerator


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

class AveragePriceAsian:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_AveragePriceAsian,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C AveragePriceAsian instance at %s>" % (self.this,)
class AveragePriceAsianPtr(AveragePriceAsian):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = AveragePriceAsian


AveragePriceAsian.value = new.instancemethod(QuantLibc.AveragePriceAsian_value, None, AveragePriceAsian)
AveragePriceAsian.errorEstimate = new.instancemethod(QuantLibc.AveragePriceAsian_errorEstimate, None, AveragePriceAsian)

class AverageStrikeAsian:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_AverageStrikeAsian,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C AverageStrikeAsian instance at %s>" % (self.this,)
class AverageStrikeAsianPtr(AverageStrikeAsian):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = AverageStrikeAsian


AverageStrikeAsian.value = new.instancemethod(QuantLibc.AverageStrikeAsian_value, None, AverageStrikeAsian)
AverageStrikeAsian.errorEstimate = new.instancemethod(QuantLibc.AverageStrikeAsian_errorEstimate, None, AverageStrikeAsian)

class PlainBasketOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_PlainBasketOption,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C PlainBasketOption instance at %s>" % (self.this,)
class PlainBasketOptionPtr(PlainBasketOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = PlainBasketOption


PlainBasketOption.value = new.instancemethod(QuantLibc.PlainBasketOption_value, None, PlainBasketOption)
PlainBasketOption.errorEstimate = new.instancemethod(QuantLibc.PlainBasketOption_errorEstimate, None, PlainBasketOption)

class Himalaya:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Himalaya,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C Himalaya instance at %s>" % (self.this,)
class HimalayaPtr(Himalaya):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Himalaya


Himalaya.value = new.instancemethod(QuantLibc.Himalaya_value, None, Himalaya)
Himalaya.errorEstimate = new.instancemethod(QuantLibc.Himalaya_errorEstimate, None, Himalaya)

class GaussianArrayGenerator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_GaussianArrayGenerator,args,kwargs)
        self.thisown = 1

    def next(*args, **kwargs):
        val = apply(QuantLibc.GaussianArrayGenerator_next,args, kwargs)
        if val: val = ArrayPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C GaussianArrayGenerator instance at %s>" % (self.this,)
class GaussianArrayGeneratorPtr(GaussianArrayGenerator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = GaussianArrayGenerator


GaussianArrayGenerator.weight = new.instancemethod(QuantLibc.GaussianArrayGenerator_weight, None, GaussianArrayGenerator)

class StandardMultiPathGenerator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_StandardMultiPathGenerator,args,kwargs)
        self.thisown = 1

    def next(*args, **kwargs):
        val = apply(QuantLibc.StandardMultiPathGenerator_next,args, kwargs)
        if val: val = MatrixPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C StandardMultiPathGenerator instance at %s>" % (self.this,)
class StandardMultiPathGeneratorPtr(StandardMultiPathGenerator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = StandardMultiPathGenerator


StandardMultiPathGenerator.weight = new.instancemethod(QuantLibc.StandardMultiPathGenerator_weight, None, StandardMultiPathGenerator)

class TridiagonalOperator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_TridiagonalOperator,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_TridiagonalOperator(self)
    def solveFor(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator_solveFor,args, kwargs)
        if val: val = ArrayPtr(val) ; val.thisown = 1
        return val
    def applyTo(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator_applyTo,args, kwargs)
        if val: val = ArrayPtr(val) ; val.thisown = 1
        return val
    def __add__(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator___add__,args, kwargs)
        if val: val = TridiagonalOperatorPtr(val) ; val.thisown = 1
        return val
    def __sub__(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator___sub__,args, kwargs)
        if val: val = TridiagonalOperatorPtr(val) ; val.thisown = 1
        return val
    def __mul__(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator___mul__,args, kwargs)
        if val: val = TridiagonalOperatorPtr(val) ; val.thisown = 1
        return val
    def __rmul__(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator___rmul__,args, kwargs)
        if val: val = TridiagonalOperatorPtr(val) ; val.thisown = 1
        return val
    def __div__(*args, **kwargs):
        val = apply(QuantLibc.TridiagonalOperator___div__,args, kwargs)
        if val: val = TridiagonalOperatorPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C TridiagonalOperator instance at %s>" % (self.this,)
class TridiagonalOperatorPtr(TridiagonalOperator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = TridiagonalOperator


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

class BinaryOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BinaryOption,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_BinaryOption(self)
    def __repr__(self):
        return "<C BinaryOption instance at %s>" % (self.this,)
class BinaryOptionPtr(BinaryOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = BinaryOption


BinaryOption.setVolatility = new.instancemethod(QuantLibc.BinaryOption_setVolatility, None, BinaryOption)
BinaryOption.setRiskFreeRate = new.instancemethod(QuantLibc.BinaryOption_setRiskFreeRate, None, BinaryOption)
BinaryOption.value = new.instancemethod(QuantLibc.BinaryOption_value, None, BinaryOption)
BinaryOption.delta = new.instancemethod(QuantLibc.BinaryOption_delta, None, BinaryOption)
BinaryOption.gamma = new.instancemethod(QuantLibc.BinaryOption_gamma, None, BinaryOption)
BinaryOption.theta = new.instancemethod(QuantLibc.BinaryOption_theta, None, BinaryOption)
BinaryOption.vega = new.instancemethod(QuantLibc.BinaryOption_vega, None, BinaryOption)
BinaryOption.rho = new.instancemethod(QuantLibc.BinaryOption_rho, None, BinaryOption)
BinaryOption.impliedVolatility = new.instancemethod(QuantLibc.BinaryOption_impliedVolatility, None, BinaryOption)

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

class BarrierOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BarrierOption,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_BarrierOption(self)
    def __repr__(self):
        return "<C BarrierOption instance at %s>" % (self.this,)
class BarrierOptionPtr(BarrierOption):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = BarrierOption


BarrierOption.value = new.instancemethod(QuantLibc.BarrierOption_value, None, BarrierOption)

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

class VarTool:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_VarTool,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C VarTool instance at %s>" % (self.this,)
class VarToolPtr(VarTool):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = VarTool


VarTool.valueAtRisk = new.instancemethod(QuantLibc.VarTool_valueAtRisk, None, VarTool)
VarTool.shortfall = new.instancemethod(QuantLibc.VarTool_shortfall, None, VarTool)
VarTool.averageShortfall = new.instancemethod(QuantLibc.VarTool_averageShortfall, None, VarTool)

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

class MultivariateAccumulator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_MultivariateAccumulator,args,kwargs)
        self.thisown = 1

    def mean(*args, **kwargs):
        val = apply(QuantLibc.MultivariateAccumulator_mean,args, kwargs)
        if val: val = ArrayPtr(val) ; val.thisown = 1
        return val
    def covariance(*args, **kwargs):
        val = apply(QuantLibc.MultivariateAccumulator_covariance,args, kwargs)
        if val: val = MatrixPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C MultivariateAccumulator instance at %s>" % (self.this,)
class MultivariateAccumulatorPtr(MultivariateAccumulator):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = MultivariateAccumulator


MultivariateAccumulator.size = new.instancemethod(QuantLibc.MultivariateAccumulator_size, None, MultivariateAccumulator)
MultivariateAccumulator.samples = new.instancemethod(QuantLibc.MultivariateAccumulator_samples, None, MultivariateAccumulator)
MultivariateAccumulator.add = new.instancemethod(QuantLibc.MultivariateAccumulator_add, None, MultivariateAccumulator)
MultivariateAccumulator.reset = new.instancemethod(QuantLibc.MultivariateAccumulator_reset, None, MultivariateAccumulator)



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

def makeCalendar(*args, **kwargs):
    val = apply(QuantLibc.makeCalendar,args,kwargs)
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

def makeCurrency(*args, **kwargs):
    val = apply(QuantLibc.makeCurrency,args,kwargs)
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

def makeDayCounter(*args, **kwargs):
    val = apply(QuantLibc.makeDayCounter,args,kwargs)
    if val: val = DayCounterPtr(val); val.thisown = 1
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

def LexicographicalView(*args, **kwargs):
    val = apply(QuantLibc.LexicographicalView,args,kwargs)
    if val: val = ArrayLexicographicalViewPtr(val); val.thisown = 1
    return val

def LinearInterpolation(*args, **kwargs):
    val = apply(QuantLibc.LinearInterpolation,args,kwargs)
    if val: val = InterpolationPtr(val); val.thisown = 1
    return val

def CubicSpline(*args, **kwargs):
    val = apply(QuantLibc.CubicSpline,args,kwargs)
    if val: val = InterpolationPtr(val); val.thisown = 1
    return val

def transpose(*args, **kwargs):
    val = apply(QuantLibc.transpose,args,kwargs)
    if val: val = MatrixPtr(val); val.thisown = 1
    return val

def outerProduct(*args, **kwargs):
    val = apply(QuantLibc.outerProduct,args,kwargs)
    if val: val = MatrixPtr(val); val.thisown = 1
    return val

def matrixProduct(*args, **kwargs):
    val = apply(QuantLibc.matrixProduct,args,kwargs)
    if val: val = MatrixPtr(val); val.thisown = 1
    return val

def matrixSqrt(*args, **kwargs):
    val = apply(QuantLibc.matrixSqrt,args,kwargs)
    if val: val = MatrixPtr(val); val.thisown = 1
    return val

def Identity(*args, **kwargs):
    val = apply(QuantLibc.Identity,args,kwargs)
    if val: val = TridiagonalOperatorPtr(val); val.thisown = 1
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

