
# $Source$

#
# makefile for QuantLib static library under Borland C++
#

.autodepend
.silent

# Directories
!ifdef DEBUG
    OUTPUT_DIR = .\Debug
!else
    OUTPUT_DIR = .\Release
!endif
SOURCES_DIR    = ..\Sources
INCLUDE_DIR    = ..\Include
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib

# Object files
CORE_OBJS        = $(OUTPUT_DIR)\calendar.obj \
                   $(OUTPUT_DIR)\dataformatters.obj \
                   $(OUTPUT_DIR)\date.obj \
                   $(OUTPUT_DIR)\ratehelper.obj \
                   $(OUTPUT_DIR)\solver1d.obj

CALENDAR_OBJS    = $(OUTPUT_DIR)\westerncalendar.obj \
                   $(OUTPUT_DIR)\frankfurt.obj \
                   $(OUTPUT_DIR)\helsinki.obj \
                   $(OUTPUT_DIR)\london.obj \
                   $(OUTPUT_DIR)\milan.obj \
                   $(OUTPUT_DIR)\newyork.obj \
                   $(OUTPUT_DIR)\target.obj \
                   $(OUTPUT_DIR)\wellington.obj \
                   $(OUTPUT_DIR)\zurich.obj

DAYCOUNT_OBJS    = $(OUTPUT_DIR)\actualactual.obj \
                   $(OUTPUT_DIR)\thirty360.obj \
                   $(OUTPUT_DIR)\thirty360italian.obj

MATH_OBJS        = $(OUTPUT_DIR)\matrix.obj      \
                   $(OUTPUT_DIR)\symmetricschurdecomposition.obj    \
                   $(OUTPUT_DIR)\multivariateaccumulator.obj\
                   $(OUTPUT_DIR)\normaldistribution.obj \
                   $(OUTPUT_DIR)\statistics.obj

MONTECARLO_OBJS  = $(OUTPUT_DIR)\avgpriceasianpathpricer.obj \
                   $(OUTPUT_DIR)\avgstrikeasianpathpricer.obj \
                   $(OUTPUT_DIR)\basketpathpricer.obj       \
                   $(OUTPUT_DIR)\controlvariatedpathpricer.obj \
                   $(OUTPUT_DIR)\europeanpathpricer.obj        \
                   $(OUTPUT_DIR)\everestpathpricer.obj        \
                   $(OUTPUT_DIR)\geometricasianpathpricer.obj  \
                   $(OUTPUT_DIR)\getcovariance.obj        \
                   $(OUTPUT_DIR)\himalayapathpricer.obj  \
                   $(OUTPUT_DIR)\lecuyerrandomgenerator.obj \
                   $(OUTPUT_DIR)\pagodapathpricer.obj

FDM_OBJS         = $(OUTPUT_DIR)\tridiagonaloperator.obj \
                   $(OUTPUT_DIR)\bsmoperator.obj\
                   $(OUTPUT_DIR)\valueatcenter.obj


PRICER_OBJS      = $(OUTPUT_DIR)\bsmoption.obj \
                   $(OUTPUT_DIR)\averagestrikeasian.obj \
                   $(OUTPUT_DIR)\averagepriceasian.obj \
                   $(OUTPUT_DIR)\barrieroption.obj \
                   $(OUTPUT_DIR)\bermudanoption.obj \
                   $(OUTPUT_DIR)\binaryoption.obj \
                   $(OUTPUT_DIR)\bsmnumericaloption.obj \
                   $(OUTPUT_DIR)\bsmeuropeanoption.obj \
                   $(OUTPUT_DIR)\dividendoption.obj \ 
                   $(OUTPUT_DIR)\dividendshoutoption.obj \ 
                   $(OUTPUT_DIR)\dividendamericanoption.obj \
                   $(OUTPUT_DIR)\dividendeuropeanoption.obj \
                   $(OUTPUT_DIR)\everestoption.obj \
                   $(OUTPUT_DIR)\finitedifferenceeuropean.obj\
                   $(OUTPUT_DIR)\himalaya.obj \                   
                   $(OUTPUT_DIR)\mceuropeanpricer.obj \
                   $(OUTPUT_DIR)\multiperiodoption.obj \
                   $(OUTPUT_DIR)\plainbasketoption.obj \
                   $(OUTPUT_DIR)\pagodaoption.obj \
                   $(OUTPUT_DIR)\stepconditionoption.obj


SOLVER1D_OBJS    = $(OUTPUT_DIR)\bisection.obj \
                   $(OUTPUT_DIR)\brent.obj \
                   $(OUTPUT_DIR)\falseposition.obj \
                   $(OUTPUT_DIR)\newton.obj \
                   $(OUTPUT_DIR)\newtonsafe.obj \
                   $(OUTPUT_DIR)\ridder.obj \
                   $(OUTPUT_DIR)\secant.obj

TERMSTRUC_OBJS   = $(OUTPUT_DIR)\piecewiseconstantforwards.obj

QUANTLIB_OBJS    = $(CORE_OBJS) \
                   $(CALENDAR_OBJS) \
                   $(DAYCOUNT_OBJS) \
                   $(MATH_OBJS) \
                   $(MONTECARLO_OBJS) \
                   $(FDM_OBJS) \
                   $(PRICER_OBJS) \
                   $(SOLVER1D_OBJS) \
                   $(TERMSTRUC_OBJS)

WIN_OBJS         = c0d32.obj

# Libraries
WIN_LIBS         = import32.lib cw32mt.lib

# Tools to be used
CC        = bcc32
TLIB      = tlib
COFF2OMF  = coff2omf

# Options
CC_OPTS        = -q -c -tWM -n$(OUTPUT_DIR) \
    -w-8026 -w-8027 -w-8012 \
    -I$(INCLUDE_DIR) \
    -I$(INCLUDE_DIR)\Calendars \
    -I$(INCLUDE_DIR)\Currencies \
    -I$(INCLUDE_DIR)\DayCounters \
    -I$(INCLUDE_DIR)\FiniteDifferences \
    -I$(INCLUDE_DIR)\Instruments \
    -I$(INCLUDE_DIR)\Math \
    -I$(INCLUDE_DIR)\MonteCarlo \
    -I$(INCLUDE_DIR)\Patterns \
    -I$(INCLUDE_DIR)\Pricers \
    -I$(INCLUDE_DIR)\Solvers1D \
    -I$(INCLUDE_DIR)\TermStructures \
    -I$(INCLUDE_DIR)\Utilities \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif

TLIB_OPTS    = /P32
!ifdef DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<

# Primary target:
# QuantLib library
QuantLib: $(OUTPUT_DIR)\QuantLib.lib

# make sure the output directory exists
$(OUTPUT_DIR):
    if not exist $(OUTPUT_DIR) md $(OUTPUT_DIR)

# QuantLib library
$(OUTPUT_DIR)\QuantLib.lib:: Core Calendars DayCounters FiniteDifferences Math MonteCarlo Pricers Solvers1D TermStructures
    if exist $(OUTPUT_DIR)\QuantLib.lib del $(OUTPUT_DIR)\QuantLib.lib
    $(TLIB) $(TLIB_OPTS) $(OUTPUT_DIR)\QuantLib.lib /a $(QUANTLIB_OBJS)

# Core
Core: $(OUTPUT_DIR) $(CORE_OBJS)
$(OUTPUT_DIR)\calendar.obj: $(SOURCES_DIR)\calendar.cpp
$(OUTPUT_DIR)\dataformatters.obj: $(SOURCES_DIR)\dataformatters.cpp
$(OUTPUT_DIR)\date.obj: $(SOURCES_DIR)\date.cpp
$(OUTPUT_DIR)\ratehelper.obj: $(SOURCES_DIR)\ratehelper.cpp
$(OUTPUT_DIR)\solver1d.obj: $(SOURCES_DIR)\solver1d.cpp


# Calendars
Calendars: $(OUTPUT_DIR) $(CALENDAR_OBJS)
$(OUTPUT_DIR)\westerncalendar.obj: $(SOURCES_DIR)\Calendars\westerncalendar.cpp
$(OUTPUT_DIR)\frankfurt.obj: $(SOURCES_DIR)\Calendars\frankfurt.cpp
$(OUTPUT_DIR)\helsinki.obj: $(SOURCES_DIR)\Calendars\helsinki.cpp
$(OUTPUT_DIR)\london.obj: $(SOURCES_DIR)\Calendars\london.cpp
$(OUTPUT_DIR)\milan.obj: $(SOURCES_DIR)\Calendars\milan.cpp
$(OUTPUT_DIR)\newyork.obj: $(SOURCES_DIR)\Calendars\newyork.cpp
$(OUTPUT_DIR)\target.obj: $(SOURCES_DIR)\Calendars\target.cpp
$(OUTPUT_DIR)\wellington.obj: $(SOURCES_DIR)\Calendars\wellington.cpp
$(OUTPUT_DIR)\zurich.obj: $(SOURCES_DIR)\Calendars\zurich.cpp


# Day counters
DayCounters: $(OUTPUT_DIR) $(DAYCOUNT_OBJS)
$(OUTPUT_DIR)\actualactual.obj: $(SOURCES_DIR)\DayCounters\actualactual.cpp
$(OUTPUT_DIR)\thirty360.obj: $(SOURCES_DIR)\DayCounters\thirty360.cpp
$(OUTPUT_DIR)\thirty360italian.obj: $(SOURCES_DIR)\DayCounters\thirty360italian.cpp


# Finite difference methods
FiniteDifferences: $(OUTPUT_DIR) $(FDM_OBJS)
$(OUTPUT_DIR)\tridiagonaloperator.obj: \
        $(SOURCES_DIR)\FiniteDifferences\tridiagonaloperator.cpp
$(OUTPUT_DIR)\bsmoperator.obj: \
        $(SOURCES_DIR)\FiniteDifferences\bsmoperator.cpp
$(OUTPUT_DIR)\valueatcenter.obj: \
        $(SOURCES_DIR)\FiniteDifferences\valueatcenter.cpp


# Math
Math: $(OUTPUT_DIR) $(MATH_OBJS)
$(OUTPUT_DIR)\symmetricschurdecomposition.obj: \
    $(SOURCES_DIR)\Math\symmetricschurdecomposition.cpp
$(OUTPUT_DIR)\matrix.obj: \
    $(SOURCES_DIR)\Math\matrix.cpp
$(OUTPUT_DIR)\normaldistribution.obj: \
    $(SOURCES_DIR)\Math\normaldistribution.cpp
$(OUTPUT_DIR)\statistics.obj: \
    $(SOURCES_DIR)\Math\statistics.cpp
$(OUTPUT_DIR)\multivariateaccumulator.obj: \
    $(SOURCES_DIR)\Math\multivariateaccumulator.cpp

# Monte Carlo
MonteCarlo: $(OUTPUT_DIR) $(MONTECARLO_OBJS)
$(OUTPUT_DIR)\avgpriceasianpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\avgpriceasianpathpricer.cpp
$(OUTPUT_DIR)\avgstrikeasianpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\avgstrikeasianpathpricer.cpp
$(OUTPUT_DIR)\basketpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\basketpathpricer.cpp
$(OUTPUT_DIR)\controlvariatedpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\controlvariatedpathpricer.cpp
$(OUTPUT_DIR)\europeanpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\europeanpathpricer.cpp
$(OUTPUT_DIR)\everestpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\everestpathpricer.cpp
$(OUTPUT_DIR)\geometricasianpathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\geometricasianpathpricer.cpp
$(OUTPUT_DIR)\getcovariance.obj: \
    $(SOURCES_DIR)\MonteCarlo\getcovariance.cpp
$(OUTPUT_DIR)\himalayapathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\himalayapathpricer.cpp
$(OUTPUT_DIR)\lecuyerrandomgenerator.obj: \
    $(SOURCES_DIR)\MonteCarlo\lecuyerrandomgenerator.cpp
$(OUTPUT_DIR)\pagodapathpricer.obj: \
    $(SOURCES_DIR)\MonteCarlo\pagodapathpricer.cpp


# Pricers
Pricers: $(OUTPUT_DIR) $(PRICER_OBJS)
$(OUTPUT_DIR)\bsmoption.obj: $(SOURCES_DIR)\Pricers\bsmoption.cpp
$(OUTPUT_DIR)\averagepriceasian.obj: \
                $(SOURCES_DIR)\Pricers\averagepriceasian.cpp
$(OUTPUT_DIR)\averagestrikeasian.obj: \
                $(SOURCES_DIR)\Pricers\averagestrikeasian.cpp
$(OUTPUT_DIR)\barrieroption.obj: \
                $(SOURCES_DIR)\Pricers\barrieroption.cpp
$(OUTPUT_DIR)\bermudanoption.obj: \
                $(SOURCES_DIR)\Pricers\bermudanoption.cpp
$(OUTPUT_DIR)\binaryoption.obj: \
                $(SOURCES_DIR)\Pricers\binaryoption.cpp
$(OUTPUT_DIR)\bsmnumericaloption.obj: \
                $(SOURCES_DIR)\Pricers\bsmnumericaloption.cpp
$(OUTPUT_DIR)\bsmeuropeanoption.obj:  \
                $(SOURCES_DIR)\Pricers\bsmeuropeanoption.cpp
$(OUTPUT_DIR)\dividendoption.obj : \
                $(SOURCES_DIR)\Pricers\dividendoption.cpp           
$(OUTPUT_DIR)\dividendshoutoption.obj : \
                $(SOURCES_DIR)\Pricers\dividendshoutoption.cpp           
$(OUTPUT_DIR)\dividendamericanoption.obj: \
                $(SOURCES_DIR)\Pricers\dividendamericanoption.cpp
$(OUTPUT_DIR)\dividendeuropeanoption.obj: \
                $(SOURCES_DIR)\Pricers\dividendeuropeanoption.cpp
$(OUTPUT_DIR)\everestoption.obj: \
                $(SOURCES_DIR)\Pricers\everestoption.cpp
$(OUTPUT_DIR)\finitedifferenceeuropean.obj: \
                $(SOURCES_DIR)\Pricers\finitedifferenceeuropean.cpp
$(OUTPUT_DIR)\himalaya.obj: \
                $(SOURCES_DIR)\Pricers\himalaya.cpp
$(OUTPUT_DIR)\mceuropeanpricer.obj: \
                $(SOURCES_DIR)\Pricers\mceuropeanpricer.cpp                
$(OUTPUT_DIR)\multiperiodoption.obj: \
                $(SOURCES_DIR)\Pricers\multiperiodoption.cpp
$(OUTPUT_DIR)\plainbasketoption.obj: \
                $(SOURCES_DIR)\Pricers\plainbasketoption.cpp
$(OUTPUT_DIR)\pagodaoption.obj: \
                $(SOURCES_DIR)\Pricers\pagodaoption.cpp                
$(OUTPUT_DIR)\stepconditionoption.obj: \
                $(SOURCES_DIR)\Pricers\stepconditionoption.cpp




# 1D solvers
Solvers1D: $(OUTPUT_DIR) $(SOLVER1D_OBJS)
$(OUTPUT_DIR)\bisection.obj: $(SOURCES_DIR)\Solvers1D\bisection.cpp
$(OUTPUT_DIR)\brent.obj: $(SOURCES_DIR)\Solvers1D\brent.cpp
$(OUTPUT_DIR)\falseposition.obj: $(SOURCES_DIR)\Solvers1D\falseposition.cpp
$(OUTPUT_DIR)\newton.obj: $(SOURCES_DIR)\Solvers1D\newton.cpp
$(OUTPUT_DIR)\newtonsafe.obj: $(SOURCES_DIR)\Solvers1D\newtonsafe.cpp
$(OUTPUT_DIR)\ridder.obj: $(SOURCES_DIR)\Solvers1D\ridder.cpp
$(OUTPUT_DIR)\secant.obj: $(SOURCES_DIR)\Solvers1D\secant.cpp


# Term structures
TermStructures: $(OUTPUT_DIR) $(TERMSTRUC_OBJS)
$(OUTPUT_DIR)\piecewiseconstantforwards.obj: \
                $(SOURCES_DIR)\TermStructures\piecewiseconstantforwards.cpp


# Clean up
clean::
    if exist .\Debug    rd /s /q .\Debug
    if exist .\Release  rd /s /q .\Release


