#
# Tentative makefile for QuantLib under Borland C++
#

# Directories
OUTPUT_DIR		= .\Release
PYTHON_DIR		= ..\Python
SWIG_DIR		= ..\Swig
SOURCES_DIR		= ..\Sources
INCLUDE_DIR		= ..\Include
PYTHON_INCLUDE	= "C:\Program Files\Python\include"
PYTHON_LIBS		= "C:\Program Files\Python\libs"
BCC_INCLUDE		= "D:\Program Files\Borland\Bcc55\include"
BCC_LIBS		= "D:\Program Files\Borland\Bcc55\lib"

# Object files
CORE_OBJS		= $(OUTPUT_DIR)\calendar.obj $(OUTPUT_DIR)\date.obj $(OUTPUT_DIR)\solver1d.obj $(OUTPUT_DIR)\statistics.obj $(OUTPUT_DIR)\dataformatters.obj
CALENDAR_OBJS	= $(OUTPUT_DIR)\westerncalendar.obj $(OUTPUT_DIR)\frankfurt.obj $(OUTPUT_DIR)\london.obj $(OUTPUT_DIR)\milan.obj $(OUTPUT_DIR)\newyork.obj $(OUTPUT_DIR)\target.obj 
DAYCOUNT_OBJS	= $(OUTPUT_DIR)\actualactual.obj $(OUTPUT_DIR)\thirty360.obj $(OUTPUT_DIR)\thirty360italian.obj
MATH_OBJS		= $(OUTPUT_DIR)\normaldistribution.obj 
FDM_OBJS		= $(OUTPUT_DIR)\tridiagonaloperator.obj $(OUTPUT_DIR)\bsmoperator.obj
PRICER_OBJS		= $(OUTPUT_DIR)\bsmoption.obj $(OUTPUT_DIR)\bsmnumericaloption.obj $(OUTPUT_DIR)\bsmeuropeanoption.obj $(OUTPUT_DIR)\bsmamericanoption.obj
SOLVER1D_OBJS	= $(OUTPUT_DIR)\bisection.obj $(OUTPUT_DIR)\brent.obj $(OUTPUT_DIR)\falseposition.obj $(OUTPUT_DIR)\newton.obj $(OUTPUT_DIR)\newtonsafe.obj $(OUTPUT_DIR)\ridder.obj $(OUTPUT_DIR)\secant.obj
TERMSTRUC_OBJS	= $(OUTPUT_DIR)\piecewiseconstantforwards.obj 
QUANTLIB_OBJS	= $(CORE_OBJS) $(CALENDAR_OBJS) $(DAYCOUNT_OBJS) $(MATH_OBJS) $(FDM_OBJS) $(PRICER_OBJS) $(SOLVER1D_OBJS) $(TERMSTRUC_OBJS) 
WIN_OBJS		= c0d32.obj 

# Libraries
WIN_LIBS 		= import32.lib cw32mt.lib
PYTHON_BCC_LIB	= bccpython.lib

# Tools to be used
CC			= bcc32
LINK		= ilink32
COFF2OMF	= coff2omf
SWIG		= swig1.3a5

# Options
CC_OPTS		= -q -c -tWM -n$(OUTPUT_DIR) -I"..\Include" -I$(PYTHON_INCLUDE) -I$(BCC_INCLUDE) -w-8027
LINK_OPTS	= -q -x -L$(BCC_LIBS)

# Generic rules
.cpp.obj:
    @$(CC) $(CC_OPTS) $<

# Primary target:
# Python module
$(PYTHON_DIR)\QuantLibc.dll:: $(OUTPUT_DIR) $(OUTPUT_DIR)\quantlib_wrap.obj QuantLib $(PYTHON_BCC_LIB)
	@echo Linking Python module...
	@$(LINK) $(LINK_OPTS) -Tpd $(OUTPUT_DIR)\quantlib_wrap.obj $(QUANTLIB_OBJS) $(WIN_OBJS),$(PYTHON_DIR)\QuantLibc.dll,, $(PYTHON_BCC_LIB) $(WIN_LIBS), QuantLibc.def
	@del $(PYTHON_DIR)\QuantLibc.ilc
	@del $(PYTHON_DIR)\QuantLibc.ild
	@del $(PYTHON_DIR)\QuantLibc.ilf
	@del $(PYTHON_DIR)\QuantLibc.ils
	@del $(PYTHON_DIR)\QuantLibc.tds
	@echo Build completed

# make sure the output directory exists
$(OUTPUT_DIR):
	@if not exist $(OUTPUT_DIR) md $(OUTPUT_DIR)

# Python lib in OMF format
$(PYTHON_BCC_LIB):
	@$(COFF2OMF) -q $(PYTHON_LIBS)\python15.lib $(PYTHON_BCC_LIB)

# Wrapper functions
$(OUTPUT_DIR)\quantlib_wrap.obj:: $(PYTHON_DIR)\quantlib_wrap.cpp
	@echo Compiling wrappers...
	@$(CC) $(CC_OPTS) -w-8057 -w-8004 -w-8060 -D__WIN32__ -DMSC_CORE_BC_EXT $(PYTHON_DIR)\quantlib_wrap.cpp
$(PYTHON_DIR)\quantlib_wrap.cpp:: $(SWIG_DIR)\QuantLib.i
	@echo Generating wrappers...
	@$(SWIG) -python -c++ -shadow -keyword -opt -I$(SWIG_DIR) -o $(PYTHON_DIR)\quantlib_wrap.cpp $(SWIG_DIR)\QuantLib.i
	@copy .\QuantLib.py $(PYTHON_DIR)\QuantLib.py
	@del .\QuantLib.py
$(SWIG_DIR)\QuantLib.i: $(INCLUDE_DIR)\qldefines.h $(SWIG_DIR)\Date.i $(SWIG_DIR)\Calendars.i $(SWIG_DIR)\DayCounters.i $(SWIG_DIR)\Currencies.i $(SWIG_DIR)\Financial.i $(SWIG_DIR)\Options.i $(SWIG_DIR)\Instruments.i $(SWIG_DIR)\Operators.i $(SWIG_DIR)\Pricers.i $(SWIG_DIR)\Solvers1D.i $(SWIG_DIR)\TermStructures.i $(SWIG_DIR)\Statistics.i
	@touch $<
$(SWIG_DIR)\Date.i: $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\dataformatters.h
	@touch $<
$(SWIG_DIR)\Calendars.i: $(SWIG_DIR)\Date.i $(INCLUDE_DIR)\calendar.h $(INCLUDE_DIR)\calendars.h
	@touch $<
$(SWIG_DIR)\DayCounters.i: $(SWIG_DIR)\Date.i $(INCLUDE_DIR)\daycounter.h $(INCLUDE_DIR)\daycounters.h
	@touch $<
$(SWIG_DIR)\Currencies.i: $(SWIG_DIR)\Date.i $(SWIG_DIR)\Calendars.i $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\currencies.h
	@touch $<
$(SWIG_DIR)\Financial.i: $(INCLUDE_DIR)\rate.h $(INCLUDE_DIR)\spread.h $(INCLUDE_DIR)\discountfactor.h
	@touch $<
$(SWIG_DIR)\Options.i: $(INCLUDE_DIR)\options.h $(INCLUDE_DIR)\dataformatters.h
	@touch $<
$(SWIG_DIR)\Instruments.i: $(SWIG_DIR)\Financial.i $(SWIG_DIR)\TermStructures.i $(INCLUDE_DIR)\instrument.h $(INCLUDE_DIR)\stock.h
	@touch $<
$(SWIG_DIR)\Operators.i: $(SWIG_DIR)\Vectors.i $(SWIG_DIR)\BoundaryConditions.i $(INCLUDE_DIR)\finitedifferences.h
	@touch $<
$(SWIG_DIR)\Pricers.i: $(SWIG_DIR)\Date.i $(SWIG_DIR)\Options.i $(SWIG_DIR)\Financial.i $(INCLUDE_DIR)\pricers.h
	@touch $<
$(SWIG_DIR)\Solvers1D.i: $(INCLUDE_DIR)\solver1d.h $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SWIG_DIR)\TermStructures.i: $(SWIG_DIR)\Date.i $(SWIG_DIR)\Calendars.i $(SWIG_DIR)\DayCounters.i $(SWIG_DIR)\Financial.i $(SWIG_DIR)\Currencies.i $(INCLUDE_DIR)\termstructure.h $(INCLUDE_DIR)\handle.h $(INCLUDE_DIR)\piecewiseconstantforwards.h 
	@touch $<
$(SWIG_DIR)\Vectors.i: $(INCLUDE_DIR)\array.h
	@touch $<
$(SWIG_DIR)\BoundaryConditions.i: $(INCLUDE_DIR)\finitedifferences.h $(INCLUDE_DIR)\dataformatters.h
	@touch $<
$(SWIG_DIR)\Statistics.i: $(SWIG_DIR)\Vectors.i $(INCLUDE_DIR)\statistics.h
	@touch $<

# QuantLib files
QuantLib: Core Calendars DayCounters FiniteDifferences Math Pricers Solvers1D TermStructures

# Core
Core: $(CORE_OBJS)
$(OUTPUT_DIR)\calendar.obj: $(SOURCES_DIR)\calendar.cpp
$(OUTPUT_DIR)\dataformatters.obj: $(SOURCES_DIR)\dataformatters.cpp
$(OUTPUT_DIR)\date.obj: $(SOURCES_DIR)\date.cpp
$(OUTPUT_DIR)\solver1d.obj: $(SOURCES_DIR)\solver1d.cpp
$(OUTPUT_DIR)\statistics.obj: $(SOURCES_DIR)\statistics.cpp

$(SOURCES_DIR)\calendar.cpp: $(INCLUDE_DIR)\calendar.h
	@touch $<
$(SOURCES_DIR)\dataformatters.cpp: $(INCLUDE_DIR)\dataformatters.h
	@touch $<
$(SOURCES_DIR)\date.cpp: $(INCLUDE_DIR)\date.h
	@touch $<
$(SOURCES_DIR)\solver1d.cpp: $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(SOURCES_DIR)\statistics.cpp: $(INCLUDE_DIR)\statistics.h
	@touch $<

$(INCLUDE_DIR)\array.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\expressiontemplates.h
	@touch $<
$(INCLUDE_DIR)\calendar.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\qlerrors.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\handle.h 
	@touch $<
$(INCLUDE_DIR)\currency.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\calendar.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\handle.h 
	@touch $<
$(INCLUDE_DIR)\dataformatters.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\null.h
	@touch $<
$(INCLUDE_DIR)\date.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\qlerrors.h
	@touch $<
$(INCLUDE_DIR)\daycounter.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\handle.h 
	@touch $<
$(INCLUDE_DIR)\discountfactor.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\qlerrors.h $(INCLUDE_DIR)\dataformatters.h 
	@touch $<
$(INCLUDE_DIR)\expressiontemplates.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\forwardvolsurface.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\rate.h $(INCLUDE_DIR)\handle.h $(INCLUDE_DIR)\observable.h
	@touch $<
$(INCLUDE_DIR)\handle.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\instrument.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\termstructure.h $(INCLUDE_DIR)\swaptionvolsurface.h $(INCLUDE_DIR)\forwardvolsurface.h
	@touch $<
$(INCLUDE_DIR)\null.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\options.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\qlerrors.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\solver1d.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\null.h $(INCLUDE_DIR)\qlerrors.h $(INCLUDE_DIR)\dataformatters.h 
	@touch $<
$(INCLUDE_DIR)\spread.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\qlerrors.h $(INCLUDE_DIR)\dataformatters.h 
	@touch $<
$(INCLUDE_DIR)\statistics.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\null.h $(INCLUDE_DIR)\qlerrors.h
	@touch $<
$(INCLUDE_DIR)\swaptionvolsurface.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\rate.h $(INCLUDE_DIR)\handle.h $(INCLUDE_DIR)\observable.h
	@touch $<
$(INCLUDE_DIR)\termstructure.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\calendar.h $(INCLUDE_DIR)\rate.h $(INCLUDE_DIR)\spread.h $(INCLUDE_DIR)\discountfactor.h  $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\handle.h $(INCLUDE_DIR)\observable.h
	@touch $<
$(INCLUDE_DIR)\rate.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\qlerrors.h $(INCLUDE_DIR)\spread.h $(INCLUDE_DIR)\dataformatters.h 
	@touch $<


# Calendars
Calendars: $(CALENDAR_OBJS)
$(OUTPUT_DIR)\westerncalendar.obj: $(SOURCES_DIR)\Calendars\westerncalendar.cpp
$(OUTPUT_DIR)\frankfurt.obj: $(SOURCES_DIR)\Calendars\frankfurt.cpp
$(OUTPUT_DIR)\london.obj: $(SOURCES_DIR)\Calendars\london.cpp
$(OUTPUT_DIR)\milan.obj: $(SOURCES_DIR)\Calendars\milan.cpp
$(OUTPUT_DIR)\newyork.obj: $(SOURCES_DIR)\Calendars\newyork.cpp
$(OUTPUT_DIR)\target.obj: $(SOURCES_DIR)\Calendars\target.cpp

$(SOURCES_DIR)\Calendars\westerncalendar.cpp: $(INCLUDE_DIR)\calendars.h
	@touch $<
$(SOURCES_DIR)\Calendars\frankfurt.cpp: $(INCLUDE_DIR)\calendars.h
	@touch $<
$(SOURCES_DIR)\Calendars\london.cpp: $(INCLUDE_DIR)\calendars.h
	@touch $<
$(SOURCES_DIR)\Calendars\milan.cpp: $(INCLUDE_DIR)\calendars.h
	@touch $<
$(SOURCES_DIR)\Calendars\newyork.cpp: $(INCLUDE_DIR)\calendars.h
	@touch $<
$(SOURCES_DIR)\Calendars\target.cpp: $(INCLUDE_DIR)\calendars.h
	@touch $<

$(INCLUDE_DIR)\calendars.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Calendars\westerncalendar.h $(INCLUDE_DIR)\Calendars\frankfurt.h $(INCLUDE_DIR)\Calendars\london.h $(INCLUDE_DIR)\Calendars\milan.h $(INCLUDE_DIR)\Calendars\newyork.h $(INCLUDE_DIR)\Calendars\target.h
	@touch $<
$(INCLUDE_DIR)\Calendars\westerncalendar.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\calendar.h 
	@touch $<
$(INCLUDE_DIR)\Calendars\frankfurt.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Calendars\westerncalendar.h 
	@touch $<
$(INCLUDE_DIR)\Calendars\london.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Calendars\westerncalendar.h 
	@touch $<
$(INCLUDE_DIR)\Calendars\milan.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Calendars\westerncalendar.h 
	@touch $<
$(INCLUDE_DIR)\Calendars\newyork.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Calendars\westerncalendar.h 
	@touch $<
$(INCLUDE_DIR)\Calendars\target.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Calendars\westerncalendar.h 
	@touch $<


# Currencies
$(INCLUDE_DIR)\currencies.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Currencies\dem.h $(INCLUDE_DIR)\Currencies\eur.h $(INCLUDE_DIR)\Currencies\gbp.h $(INCLUDE_DIR)\Currencies\itl.h $(INCLUDE_DIR)\Currencies\usd.h
	@touch $<
$(INCLUDE_DIR)\Currencies\dem.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\calendars.h
	@touch $<
$(INCLUDE_DIR)\Currencies\eur.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\calendars.h
	@touch $<
$(INCLUDE_DIR)\Currencies\gbp.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\calendars.h
	@touch $<
$(INCLUDE_DIR)\Currencies\itl.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\calendars.h
	@touch $<
$(INCLUDE_DIR)\Currencies\usd.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\currency.h $(INCLUDE_DIR)\calendars.h
	@touch $<


# Day counters
DayCounters: $(DAYCOUNT_OBJS)
$(OUTPUT_DIR)\actualactual.obj: $(SOURCES_DIR)\DayCounters\actualactual.cpp
$(OUTPUT_DIR)\thirty360.obj: $(SOURCES_DIR)\DayCounters\thirty360.cpp
$(OUTPUT_DIR)\thirty360italian.obj: $(SOURCES_DIR)\DayCounters\thirty360italian.cpp

$(SOURCES_DIR)\DayCounters\actualactual.cpp: $(INCLUDE_DIR)\daycounters.h $(INCLUDE_DIR)\null.h
	@touch $<
$(SOURCES_DIR)\DayCounters\thirty360.cpp: $(INCLUDE_DIR)\daycounters.h
	@touch $<
$(SOURCES_DIR)\DayCounters\thirty360italian.cpp: $(INCLUDE_DIR)\daycounters.h
	@touch $<

$(INCLUDE_DIR)\daycounters.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\DayCounters\actual360.h $(INCLUDE_DIR)\DayCounters\actual365.h $(INCLUDE_DIR)\DayCounters\actualactual.h $(INCLUDE_DIR)\DayCounters\thirty360.h $(INCLUDE_DIR)\DayCounters\thirty360european.h $(INCLUDE_DIR)\DayCounters\thirty360italian.h
	@touch $<
$(INCLUDE_DIR)\DayCounters\actual360.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\daycounter.h
	@touch $<
$(INCLUDE_DIR)\DayCounters\actual365.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\daycounter.h
	@touch $<
$(INCLUDE_DIR)\DayCounters\actualactual.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\daycounter.h
	@touch $<
$(INCLUDE_DIR)\DayCounters\thirty360.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\daycounter.h
	@touch $<
$(INCLUDE_DIR)\DayCounters\thirty360european.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\daycounter.h
	@touch $<
$(INCLUDE_DIR)\DayCounters\thirty360italian.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\daycounter.h
	@touch $<


# Finite difference methods
FiniteDifferences: $(FDM_OBJS)
$(OUTPUT_DIR)\tridiagonaloperator.obj: $(SOURCES_DIR)\FiniteDifferences\tridiagonaloperator.cpp
$(OUTPUT_DIR)\bsmoperator.obj: $(SOURCES_DIR)\FiniteDifferences\bsmoperator.cpp

$(SOURCES_DIR)\FiniteDifferences\tridiagonaloperator.cpp: $(INCLUDE_DIR)\finitedifferences.h
	@touch $<
$(SOURCES_DIR)\FiniteDifferences\bsmoperator.cpp: $(INCLUDE_DIR)\finitedifferences.h
	@touch $<

$(INCLUDE_DIR)\finitedifferences.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\FiniteDifferences\backwardeuler.h $(INCLUDE_DIR)\FiniteDifferences\bsmoperator.h $(INCLUDE_DIR)\FiniteDifferences\boundarycondition.h $(INCLUDE_DIR)\FiniteDifferences\cranknicolson.h $(INCLUDE_DIR)\FiniteDifferences\finitedifferencemodel.h $(INCLUDE_DIR)\FiniteDifferences\forwardeuler.h $(INCLUDE_DIR)\FiniteDifferences\identity.h $(INCLUDE_DIR)\FiniteDifferences\operator.h $(INCLUDE_DIR)\FiniteDifferences\operatortraits.h $(INCLUDE_DIR)\FiniteDifferences\stepcondition.h $(INCLUDE_DIR)\FiniteDifferences\tridiagonaloperator.h
$(INCLUDE_DIR)\FiniteDifferences\backwardeuler.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\FiniteDifferences\identity.h $(INCLUDE_DIR)\FiniteDifferences\operatortraits.h 
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\bsmoperator.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\FiniteDifferences\tridiagonaloperator.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\boundarycondition.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\null.h $(INCLUDE_DIR)\qlerrors.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\cranknicolson.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\FiniteDifferences\identity.h $(INCLUDE_DIR)\FiniteDifferences\operatortraits.h 
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\finitedifferencemodel.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\handle.h $(INCLUDE_DIR)\null.h  $(INCLUDE_DIR)\FiniteDifferences\stepcondition.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\forwardeuler.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\FiniteDifferences\identity.h $(INCLUDE_DIR)\FiniteDifferences\operatortraits.h 
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\identity.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\operator.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\operatortraits.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\array.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\stepcondition.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h
	@touch $<
$(INCLUDE_DIR)\FiniteDifferences\tridiagonaloperator.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\array.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\FiniteDifferences\operator.h $(INCLUDE_DIR)\FiniteDifferences\identity.h $(INCLUDE_DIR)\FiniteDifferences\boundarycondition.h
	@touch $<


# Instruments
$(INCLUDE_DIR)\stock.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\instrument.h
	@touch $<


# Math
Math: $(MATH_OBJS)
$(OUTPUT_DIR)\normaldistribution.obj: $(SOURCES_DIR)\Math\normaldistribution.cpp

$(SOURCES_DIR)\Math\normaldistribution.cpp: $(INCLUDE_DIR)\normaldistribution.h
	@touch $<

$(INCLUDE_DIR)\normaldistribution.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<


# Patterns
$(INCLUDE_DIR)\observable.h: $(INCLUDE_DIR)\qldefines.h
	@touch $<


# Pricers
Pricers: $(PRICER_OBJS)
$(OUTPUT_DIR)\bsmoption.obj: $(SOURCES_DIR)\Pricers\bsmoption.cpp
$(OUTPUT_DIR)\bsmnumericaloption.obj: $(SOURCES_DIR)\Pricers\bsmnumericaloption.cpp
$(OUTPUT_DIR)\bsmeuropeanoption.obj: $(SOURCES_DIR)\Pricers\bsmeuropeanoption.cpp
$(OUTPUT_DIR)\bsmamericanoption.obj: $(SOURCES_DIR)\Pricers\bsmamericanoption.cpp

$(SOURCES_DIR)\Pricers\bsmoption.cpp: $(INCLUDE_DIR)\Pricers\bsmoption.h $(INCLUDE_DIR)\qlerrors.h $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Pricers\bsmnumericaloption.cpp: $(INCLUDE_DIR)\Pricers\bsmnumericaloption.h $(INCLUDE_DIR)\qlerrors.h
	@touch $<
$(SOURCES_DIR)\Pricers\bsmeuropeanoption.cpp: $(INCLUDE_DIR)\discountfactor.h $(INCLUDE_DIR)\normaldistribution.h $(INCLUDE_DIR)\Pricers\bsmeuropeanoption.h
	@touch $<
$(SOURCES_DIR)\Pricers\bsmamericanoption.cpp: $(INCLUDE_DIR)\finitedifferences.h $(INCLUDE_DIR)\Pricers\americancondition.h $(INCLUDE_DIR)\Pricers\bsmamericanoption.h $(INCLUDE_DIR)\Pricers\bsmeuropeanoption.h 
	@touch $<

$(INCLUDE_DIR)\pricers.h: $(INCLUDE_DIR)\Pricers\bsmoption.h $(INCLUDE_DIR)\Pricers\bsmnumericaloption.h $(INCLUDE_DIR)\Pricers\bsmeuropeanoption.h $(INCLUDE_DIR)\Pricers\bsmamericanoption.h
$(INCLUDE_DIR)\Pricers\americancondition.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Pricers\bsmnumericaloption.h $(INCLUDE_DIR)\finitedifferences.h
	@touch $<
$(INCLUDE_DIR)\Pricers\bsmoption.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\options.h $(INCLUDE_DIR)\rate.h
	@touch $<
$(INCLUDE_DIR)\Pricers\bsmnumericaloption.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Pricers\bsmoption.h $(INCLUDE_DIR)\array.h $(INCLUDE_DIR)\finitedifferences.h
	@touch $<
$(INCLUDE_DIR)\Pricers\bsmeuropeanoption.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Pricers\bsmoption.h
	@touch $<
$(INCLUDE_DIR)\Pricers\bsmamericanoption.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Pricers\bsmnumericaloption.h $(INCLUDE_DIR)\finitedifferences.h $(INCLUDE_DIR)\Pricers\americancondition.h
	@touch $<

# 1D solvers
Solvers1D: $(SOLVER1D_OBJS)
$(OUTPUT_DIR)\bisection.obj: $(SOURCES_DIR)\Solvers1D\bisection.cpp
$(OUTPUT_DIR)\brent.obj: $(SOURCES_DIR)\Solvers1D\brent.cpp
$(OUTPUT_DIR)\falseposition.obj: $(SOURCES_DIR)\Solvers1D\falseposition.cpp
$(OUTPUT_DIR)\newton.obj: $(SOURCES_DIR)\Solvers1D\newton.cpp
$(OUTPUT_DIR)\newtonsafe.obj: $(SOURCES_DIR)\Solvers1D\newtonsafe.cpp
$(OUTPUT_DIR)\ridder.obj: $(SOURCES_DIR)\Solvers1D\ridder.cpp
$(OUTPUT_DIR)\secant.obj: $(SOURCES_DIR)\Solvers1D\secant.cpp

$(SOURCES_DIR)\Solvers1D\bisection.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Solvers1D\brent.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Solvers1D\falseposition.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Solvers1D\newton.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Solvers1D\newtonsafe.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Solvers1D\ridder.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<
$(SOURCES_DIR)\Solvers1D\secant.cpp: $(INCLUDE_DIR)\solvers1d.h
	@touch $<

$(INCLUDE_DIR)\solvers1d.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\Solvers1D\bisection.h $(INCLUDE_DIR)\Solvers1D\brent.h $(INCLUDE_DIR)\Solvers1D\falseposition.h $(INCLUDE_DIR)\Solvers1D\newton.h $(INCLUDE_DIR)\Solvers1D\newtonsafe.h $(INCLUDE_DIR)\Solvers1D\ridder.h $(INCLUDE_DIR)\Solvers1D\secant.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\bisection.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\brent.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\falseposition.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\newton.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\newtonsafe.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\ridder.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<
$(INCLUDE_DIR)\Solvers1D\secant.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\solver1d.h
	@touch $<


# Term structures
TermStructures: $(TERMSTRUC_OBJS)
$(OUTPUT_DIR)\piecewiseconstantforwards.obj: $(SOURCES_DIR)\TermStructures\piecewiseconstantforwards.cpp

$(SOURCES_DIR)\TermStructures\piecewiseconstantforwards.cpp: $(INCLUDE_DIR)\piecewiseconstantforwards.h
	@touch $<

$(INCLUDE_DIR)\deposit.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\calendar.h $(INCLUDE_DIR)\date.h $(INCLUDE_DIR)\daycounter.h $(INCLUDE_DIR)\rate.h
	@touch $<
$(INCLUDE_DIR)\piecewiseconstantforwards.h: $(INCLUDE_DIR)\qldefines.h $(INCLUDE_DIR)\termstructure.h $(INCLUDE_DIR)\deposit.h
	@touch $<
