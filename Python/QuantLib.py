# This file was created automatically by SWIG.
import QuantLibc
import new
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


Date.dayOfWeek = new.instancemethod(QuantLibc.Date_dayOfWeek, None, Date)
Date.dayOfMonth = new.instancemethod(QuantLibc.Date_dayOfMonth, None, Date)
Date.dayOfYear = new.instancemethod(QuantLibc.Date_dayOfYear, None, Date)
Date.month = new.instancemethod(QuantLibc.Date_month, None, Date)
Date.year = new.instancemethod(QuantLibc.Date_year, None, Date)
Date.serialNumber = new.instancemethod(QuantLibc.Date_serialNumber, None, Date)
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
class TermStructurePtr(TermStructure):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = TermStructure


TermStructure.zeroYield = new.instancemethod(QuantLibc.TermStructure_zeroYield, None, TermStructure)
TermStructure.discount = new.instancemethod(QuantLibc.TermStructure_discount, None, TermStructure)
TermStructure.forward = new.instancemethod(QuantLibc.TermStructure_forward, None, TermStructure)
TermStructure.__str__ = new.instancemethod(QuantLibc.TermStructure___str__, None, TermStructure)
TermStructure.__repr__ = new.instancemethod(QuantLibc.TermStructure___repr__, None, TermStructure)
TermStructure.__nonzero__ = new.instancemethod(QuantLibc.TermStructure___nonzero__, None, TermStructure)

class Deposit:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_Deposit,args,kwargs)
        self.thisown = 1

    def __del__(self,QuantLibc=QuantLibc):
        if self.thisown == 1 :
            QuantLibc.delete_Deposit(self)
    def maturity(*args, **kwargs):
        val = apply(QuantLibc.Deposit_maturity,args, kwargs)
        if val: val = DatePtr(val) ; val.thisown = 1
        return val
    def dayCounter(*args, **kwargs):
        val = apply(QuantLibc.Deposit_dayCounter,args, kwargs)
        if val: val = DayCounterPtr(val) ; val.thisown = 1
        return val
    def __repr__(self):
        return "<C Deposit instance at %s>" % (self.this,)
class DepositPtr(Deposit):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Deposit


Deposit.rate = new.instancemethod(QuantLibc.Deposit_rate, None, Deposit)

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

class BoundaryCondition:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BoundaryCondition,args,kwargs)
        self.thisown = 1

    def __repr__(self):
        return "<C BoundaryCondition instance at %s>" % (self.this,)
class BoundaryConditionPtr(BoundaryCondition):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = BoundaryCondition


BoundaryCondition.type = new.instancemethod(QuantLibc.BoundaryCondition_type, None, BoundaryCondition)
BoundaryCondition.value = new.instancemethod(QuantLibc.BoundaryCondition_value, None, BoundaryCondition)

class TridiagonalOperator:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_TridiagonalOperator,args,kwargs)
        self.thisown = 1

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

class BSMEuropeanOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_BSMEuropeanOption,args,kwargs)
        self.thisown = 1

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
DividendAmericanOption.vega = new.instancemethod(QuantLibc.DividendAmericanOption_vega, None, DividendAmericanOption)
DividendAmericanOption.rho = new.instancemethod(QuantLibc.DividendAmericanOption_rho, None, DividendAmericanOption)
DividendAmericanOption.impliedVolatility = new.instancemethod(QuantLibc.DividendAmericanOption_impliedVolatility, None, DividendAmericanOption)

class DividendEuropeanOption:
    def __init__(self,*args,**kwargs):
        self.this = apply(QuantLibc.new_DividendEuropeanOption,args,kwargs)
        self.thisown = 1

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
DividendEuropeanOption.vega = new.instancemethod(QuantLibc.DividendEuropeanOption_vega, None, DividendEuropeanOption)
DividendEuropeanOption.impliedVolatility = new.instancemethod(QuantLibc.DividendEuropeanOption_impliedVolatility, None, DividendEuropeanOption)

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

def ImpliedTermStructure(*args, **kwargs):
    val = apply(QuantLibc.ImpliedTermStructure,args,kwargs)
    if val: val = TermStructurePtr(val); val.thisown = 1
    return val

def SpreadedTermStructure(*args, **kwargs):
    val = apply(QuantLibc.SpreadedTermStructure,args,kwargs)
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

