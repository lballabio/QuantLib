
.autodepend
.silent

MAKE = $(MAKE)

!ifdef _DEBUG
!ifndef _RTLDLL
    _D = -sd
!else
    _D = -d
!endif
!else
!ifndef _RTLDLL
    _D = -s
!endif
!endif

!ifdef __MT__
    _mt = -mt
!endif

# Directories
INCLUDE_DIR    = ..
OUTPUT_DIR     = ..\lib

# Object files
CORE_OBJS = \
    "calendar.obj$(_mt)$(_D)" \
    "currency.obj$(_mt)$(_D)" \
    "date.obj$(_mt)$(_D)" \
    "discretizedasset.obj$(_mt)$(_D)" \
    "errors.obj$(_mt)$(_D)" \
    "exchangerate.obj$(_mt)$(_D)" \
    "exercise.obj$(_mt)$(_D)" \
    "interestrate.obj$(_mt)$(_D)" \
    "money.obj$(_mt)$(_D)" \
    "schedule.obj$(_mt)$(_D)" \
    "stochasticprocess.obj$(_mt)$(_D)" \
    "timegrid.obj$(_mt)$(_D)" \
    "voltermstructure.obj$(_mt)$(_D)"

CALENDAR_LIB         = "Calendars\Calendars$(_mt)$(_D).lib"
CASHFLOWS_LIB        = "CashFlows\CashFlows$(_mt)$(_D).lib"
CURRENCIES_LIB       = "Currencies\Currencies$(_mt)$(_D).lib"
DAYCOUNTER_LIB       = "DayCounters\DayCounters$(_mt)$(_D).lib"
FDM_LIB              = "FiniteDifferences\FiniteDifferences$(_mt)$(_D).lib"
INDEXES_LIB          = "Indexes\Indexes$(_mt)$(_D).lib"
INSTRUMENTS_LIB      = "Instruments\Instruments$(_mt)$(_D).lib"
LATTICES_LIB         = "Lattices\Lattices$(_mt)$(_D).lib"
MATH_LIB             = "Math\Math$(_mt)$(_D).lib"
MONTECARLO_LIB       = "MonteCarlo\MonteCarlo$(_mt)$(_D).lib"
OPTIMIZATION_LIB     = "Optimization\Optimization$(_mt)$(_D).lib"
PRICER_LIB           = "Pricers\Pricers$(_mt)$(_D).lib"
PRICING_ENGINES_LIB  = "PricingEngines\PricingEngines$(_mt)$(_D).lib"
PROCESSES_LIB        = "Processes\Processes$(_mt)$(_D).lib"
RNG_LIB              = "RandomNumbers\RandomNumbers$(_mt)$(_D).lib"
SHORTRATEMODELS_LIB  = "ShortRateModels\ShortRateModels$(_mt)$(_D).lib"
TERMSTRUCT_LIB       = "TermStructures\TermStructures$(_mt)$(_D).lib"
UTILITIES_LIB        = "Utilities\Utilities$(_mt)$(_D).lib"
VOLATILITY_LIB       = "Volatilities\Volatilities$(_mt)$(_D).lib"

QUANTLIB_OBJS = \
    $(CALENDAR_LIB) \
    $(CASHFLOWS_LIB) \
    $(CURRENCIES_LIB) \
    $(CORE_OBJS) \
    $(DAYCOUNTER_LIB) \
    $(FDM_LIB) \
    $(INDEXES_LIB) \
    $(INSTRUMENTS_LIB) \
    $(LATTICES_LIB) \
    $(MATH_LIB) \
    $(MONTECARLO_LIB) \
    $(OPTIMIZATION_LIB) \
    $(PRICER_LIB) \
    $(PRICING_ENGINES_LIB) \
    $(PROCESSES_LIB) \
    $(RNG_LIB) \
    $(SHORTRATEMODELS_LIB) \
    $(TERMSTRUCT_LIB) \
    $(UTILITIES_LIB) \
    $(VOLATILITY_LIB)

# Tools to be used
CC        = bcc32
TLIB      = tlib

# MAKE Options
!ifdef __MT__
    MAKE = $(MAKE) -D__MT__
!endif
!ifdef _RTLDLL
    MAKE = $(MAKE) -D_RTLDLL
!endif
!ifdef _DEBUG
    MAKE = $(MAKE) -D_DEBUG
!endif
!ifdef SAFE
    MAKE = $(MAKE) -DSAFE
!endif


# Options
CC_OPTS = -vi- -q -c -I$(INCLUDE_DIR) -w-8070

!ifdef _DEBUG
    CC_OPTS = $(CC_OPTS) -v -D_DEBUG
!else
    CC_OPTS = $(CC_OPTS) -O2 -DNDEBUG
!endif

!ifdef _RTLDLL
    CC_OPTS = $(CC_OPTS) -D_RTLDLL
!endif

!ifdef __MT__
    CC_OPTS = $(CC_OPTS) -tWM
!endif

!ifdef SAFE
    CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif



TLIB_OPTS    = /P512
!ifdef _DEBUG
TLIB_OPTS    = /P1024
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# QuantLib library
$(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-$(VERSION_STRING).lib:: $(OUTPUT_DIR) \
                                                     $(CORE_OBJS) SubLibraries
    if exist $(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-$(VERSION_STRING).lib \
         del $(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-$(VERSION_STRING).lib
    $(TLIB) $(TLIB_OPTS) \
        "$(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-$(VERSION_STRING).lib" \
        /a $(QUANTLIB_OBJS)

$(OUTPUT_DIR):
    if not exist ..\lib md ..\lib

SubLibraries:
    cd Calendars
    $(MAKE)
    cd ..\CashFlows
    $(MAKE)
    cd ..\Currencies
    $(MAKE)
    cd ..\DayCounters
    $(MAKE)
    cd ..\FiniteDifferences
    $(MAKE)
    cd ..\Indexes
    $(MAKE)
    cd ..\Instruments
    $(MAKE)
    cd ..\Lattices
    $(MAKE)
    cd ..\Math
    $(MAKE)
    cd ..\MonteCarlo
    $(MAKE)
    cd ..\Optimization
    $(MAKE)
    cd ..\Pricers
    $(MAKE)
    cd ..\PricingEngines
    $(MAKE)
    cd ..\Processes
    $(MAKE)
    cd ..\RandomNumbers
    $(MAKE)
    cd ..\ShortRateModels
    $(MAKE)
    cd ..\TermStructures
    $(MAKE)
    cd ..\Utilities
    $(MAKE)
    cd ..\Volatilities
    $(MAKE)
    cd ..


# Clean up
clean::
    cd Calendars
    $(MAKE) clean
    cd ..\CashFlows
    $(MAKE) clean
    cd ..\Currencies
    $(MAKE) clean
    cd ..\DayCounters
    $(MAKE) clean
    cd ..\FiniteDifferences
    $(MAKE) clean
    cd ..\Indexes
    $(MAKE) clean
    cd ..\Instruments
    $(MAKE) clean
    cd ..\Lattices
    $(MAKE) clean
    cd ..\Math
    $(MAKE) clean
    cd ..\MonteCarlo
    $(MAKE) clean
    cd ..\Optimization
    $(MAKE) clean
    cd ..\Pricers
    $(MAKE) clean
    cd ..\PricingEngines
    $(MAKE) clean
    cd ..\Processes
    $(MAKE) clean
    cd ..\RandomNumbers
    $(MAKE) clean
    cd ..\ShortRateModels
    $(MAKE) clean
    cd ..\TermStructures
    $(MAKE) clean
    cd ..\Volatilities
    $(MAKE) clean
    cd ..
    if exist *.obj*                    del /q *.obj*
    if exist *.lib                     del /q *.lib
    if exist $(OUTPUT_DIR)\*-bcb-*.lib del /q $(OUTPUT_DIR)\*-bcb-*.lib

