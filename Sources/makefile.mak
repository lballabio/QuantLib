
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
$(OUTPUT_DIR)\calendar.obj:       calendar.cpp
$(OUTPUT_DIR)\dataformatters.obj: dataformatters.cpp
$(OUTPUT_DIR)\date.obj:           date.cpp
$(OUTPUT_DIR)\ratehelper.obj:     ratehelper.cpp
$(OUTPUT_DIR)\solver1d.obj:       solver1d.cpp


# Calendars
Calendars: $(OUTPUT_DIR) $(CALENDAR_OBJS)
$(OUTPUT_DIR)\westerncalendar.obj: Calendars\westerncalendar.cpp
$(OUTPUT_DIR)\frankfurt.obj:       Calendars\frankfurt.cpp
$(OUTPUT_DIR)\helsinki.obj:        Calendars\helsinki.cpp
$(OUTPUT_DIR)\london.obj:          Calendars\london.cpp
$(OUTPUT_DIR)\milan.obj:           Calendars\milan.cpp
$(OUTPUT_DIR)\newyork.obj:         Calendars\newyork.cpp
$(OUTPUT_DIR)\target.obj:          Calendars\target.cpp
$(OUTPUT_DIR)\wellington.obj:      Calendars\wellington.cpp
$(OUTPUT_DIR)\zurich.obj:          Calendars\zurich.cpp


# Day counters
DayCounters: $(OUTPUT_DIR) $(DAYCOUNT_OBJS)
$(OUTPUT_DIR)\actualactual.obj:     DayCounters\actualactual.cpp
$(OUTPUT_DIR)\thirty360.obj:        DayCounters\thirty360.cpp
$(OUTPUT_DIR)\thirty360italian.obj: DayCounters\thirty360italian.cpp


# Finite difference methods
FiniteDifferences: $(OUTPUT_DIR) $(FDM_OBJS)
$(OUTPUT_DIR)\tridiagonaloperator.obj: FiniteDifferences\tridiagonaloperator.cpp
$(OUTPUT_DIR)\bsmoperator.obj:         FiniteDifferences\bsmoperator.cpp
$(OUTPUT_DIR)\valueatcenter.obj:       FiniteDifferences\valueatcenter.cpp


# Math
Math: $(OUTPUT_DIR) $(MATH_OBJS)
$(OUTPUT_DIR)\symmetricschurdecomposition.obj: \
                                           Math\symmetricschurdecomposition.cpp
$(OUTPUT_DIR)\matrix.obj:                  Math\matrix.cpp
$(OUTPUT_DIR)\normaldistribution.obj:      Math\normaldistribution.cpp
$(OUTPUT_DIR)\statistics.obj:              Math\statistics.cpp
$(OUTPUT_DIR)\multivariateaccumulator.obj: Math\multivariateaccumulator.cpp

# Monte Carlo
MonteCarlo: $(OUTPUT_DIR) $(MONTECARLO_OBJS)
$(OUTPUT_DIR)\avgpriceasianpathpricer.obj: \
    MonteCarlo\avgpriceasianpathpricer.cpp
$(OUTPUT_DIR)\avgstrikeasianpathpricer.obj: \
    MonteCarlo\avgstrikeasianpathpricer.cpp
$(OUTPUT_DIR)\basketpathpricer.obj: \
    MonteCarlo\basketpathpricer.cpp
$(OUTPUT_DIR)\controlvariatedpathpricer.obj: \
    MonteCarlo\controlvariatedpathpricer.cpp
$(OUTPUT_DIR)\europeanpathpricer.obj: \
    MonteCarlo\europeanpathpricer.cpp
$(OUTPUT_DIR)\everestpathpricer.obj: \
    MonteCarlo\everestpathpricer.cpp
$(OUTPUT_DIR)\geometricasianpathpricer.obj: \
    MonteCarlo\geometricasianpathpricer.cpp
$(OUTPUT_DIR)\getcovariance.obj: \
    MonteCarlo\getcovariance.cpp
$(OUTPUT_DIR)\himalayapathpricer.obj: \
    MonteCarlo\himalayapathpricer.cpp
$(OUTPUT_DIR)\lecuyerrandomgenerator.obj: \
    MonteCarlo\lecuyerrandomgenerator.cpp
$(OUTPUT_DIR)\pagodapathpricer.obj: \
    MonteCarlo\pagodapathpricer.cpp


# Pricers
Pricers: $(OUTPUT_DIR) $(PRICER_OBJS)
$(OUTPUT_DIR)\bsmoption.obj: Pricers\bsmoption.cpp
$(OUTPUT_DIR)\averagepriceasian.obj: \
                Pricers\averagepriceasian.cpp
$(OUTPUT_DIR)\averagestrikeasian.obj: \
                Pricers\averagestrikeasian.cpp
$(OUTPUT_DIR)\barrieroption.obj: \
                Pricers\barrieroption.cpp
$(OUTPUT_DIR)\bermudanoption.obj: \
                Pricers\bermudanoption.cpp
$(OUTPUT_DIR)\binaryoption.obj: \
                Pricers\binaryoption.cpp
$(OUTPUT_DIR)\bsmnumericaloption.obj: \
                Pricers\bsmnumericaloption.cpp
$(OUTPUT_DIR)\bsmeuropeanoption.obj:  \
                Pricers\bsmeuropeanoption.cpp
$(OUTPUT_DIR)\dividendoption.obj : \
                Pricers\dividendoption.cpp           
$(OUTPUT_DIR)\dividendshoutoption.obj : \
                Pricers\dividendshoutoption.cpp           
$(OUTPUT_DIR)\dividendamericanoption.obj: \
                Pricers\dividendamericanoption.cpp
$(OUTPUT_DIR)\dividendeuropeanoption.obj: \
                Pricers\dividendeuropeanoption.cpp
$(OUTPUT_DIR)\everestoption.obj: \
                Pricers\everestoption.cpp
$(OUTPUT_DIR)\finitedifferenceeuropean.obj: \
                Pricers\finitedifferenceeuropean.cpp
$(OUTPUT_DIR)\himalaya.obj: \
                Pricers\himalaya.cpp
$(OUTPUT_DIR)\mceuropeanpricer.obj: \
                Pricers\mceuropeanpricer.cpp                
$(OUTPUT_DIR)\multiperiodoption.obj: \
                Pricers\multiperiodoption.cpp
$(OUTPUT_DIR)\plainbasketoption.obj: \
                Pricers\plainbasketoption.cpp
$(OUTPUT_DIR)\pagodaoption.obj: \
                Pricers\pagodaoption.cpp                
$(OUTPUT_DIR)\stepconditionoption.obj: \
                Pricers\stepconditionoption.cpp




# 1D solvers
Solvers1D: $(OUTPUT_DIR) $(SOLVER1D_OBJS)
$(OUTPUT_DIR)\bisection.obj:     Solvers1D\bisection.cpp
$(OUTPUT_DIR)\brent.obj:         Solvers1D\brent.cpp
$(OUTPUT_DIR)\falseposition.obj: Solvers1D\falseposition.cpp
$(OUTPUT_DIR)\newton.obj:        Solvers1D\newton.cpp
$(OUTPUT_DIR)\newtonsafe.obj:    Solvers1D\newtonsafe.cpp
$(OUTPUT_DIR)\ridder.obj:        Solvers1D\ridder.cpp
$(OUTPUT_DIR)\secant.obj:        Solvers1D\secant.cpp


# Term structures
TermStructures: $(OUTPUT_DIR) $(TERMSTRUC_OBJS)
$(OUTPUT_DIR)\piecewiseconstantforwards.obj: \
                TermStructures\piecewiseconstantforwards.cpp


# Clean up
clean::
    if exist .\Debug    rd /s /q .\Debug
    if exist .\Release  rd /s /q .\Release


