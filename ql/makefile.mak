
.autodepend
#.silent

MAKE = $(MAKE) -fmakefile.mak

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
    "businessdayconvention.obj$(_mt)$(_D)" \
    "calendar.obj$(_mt)$(_D)" \
    "currency.obj$(_mt)$(_D)" \
    "date.obj$(_mt)$(_D)" \
    "discretizedasset.obj$(_mt)$(_D)" \
    "errors.obj$(_mt)$(_D)" \
    "exchangerate.obj$(_mt)$(_D)" \
    "exercise.obj$(_mt)$(_D)" \
    "index.obj$(_mt)$(_D)" \
    "interestrate.obj$(_mt)$(_D)" \
    "money.obj$(_mt)$(_D)" \
    "period.obj$(_mt)$(_D)" \
    "prices.obj$(_mt)$(_D)" \
    "schedule.obj$(_mt)$(_D)" \
    "stochasticprocess.obj$(_mt)$(_D)" \
    "timegrid.obj$(_mt)$(_D)" \
    "voltermstructure.obj$(_mt)$(_D)"

CALENDAR_LIB         = "calendars\Calendars$(_mt)$(_D).lib"
CASHFLOWS_LIB        = "cashflows\CashFlows$(_mt)$(_D).lib"
CURRENCIES_LIB       = "currencies\Currencies$(_mt)$(_D).lib"
DAYCOUNTER_LIB       = "daycounters\DayCounters$(_mt)$(_D).lib"
FDM_LIB              = "finitedifferences\FiniteDifferences$(_mt)$(_D).lib"
INDEXES_LIB          = "indexes\Indexes$(_mt)$(_D).lib"
INSTRUMENTS_LIB      = "instruments\Instruments$(_mt)$(_D).lib"
LATTICES_LIB         = "lattices\Lattices$(_mt)$(_D).lib"
MARKETMODELS_LIB     = "marketmodels\MarketModels$(_mt)$(_D).lib"
MATH_LIB             = "math\Math$(_mt)$(_D).lib"
MONTECARLO_LIB       = "montecarlo\MonteCarlo$(_mt)$(_D).lib"
OPTIMIZATION_LIB     = "optimization\Optimization$(_mt)$(_D).lib"
PRICER_LIB           = "pricers\Pricers$(_mt)$(_D).lib"
PRICING_ENGINES_LIB  = "pricingengines\PricingEngines$(_mt)$(_D).lib"
PROCESSES_LIB        = "processes\Processes$(_mt)$(_D).lib"
RNG_LIB              = "randomnumbers\RandomNumbers$(_mt)$(_D).lib"
SHORTRATEMODELS_LIB  = "shortratemodels\ShortRateModels$(_mt)$(_D).lib"
TERMSTRUCT_LIB       = "termstructures\TermStructures$(_mt)$(_D).lib"
UTILITIES_LIB        = "utilities\Utilities$(_mt)$(_D).lib"
VOLATILITY_LIB       = "volatilities\Volatilities$(_mt)$(_D).lib"
VOLMODELS_LIB        = "volatilitymodels\VolatilityModels$(_mt)$(_D).lib"

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
    $(MARKETMODELS_LIB) \
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
    $(VOLATILITY_LIB) \
    $(VOLMODELS_LIB)

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
    cd calendars
    $(MAKE)
    cd ..\cashflows
    $(MAKE)
    cd ..\currencies
    $(MAKE)
    cd ..\daycounters
    $(MAKE)
    cd ..\finitedifferences
    $(MAKE)
    cd ..\indexes
    $(MAKE)
    cd ..\instruments
    $(MAKE)
    cd ..\lattices
    $(MAKE)
    cd ..\marketmodels
    $(MAKE)
    cd ..\math
    $(MAKE)
    cd ..\montecarlo
    $(MAKE)
    cd ..\optimization
    $(MAKE)
    cd ..\pricers
    $(MAKE)
    cd ..\pricingengines
    $(MAKE)
    cd ..\processes
    $(MAKE)
    cd ..\randomnumbers
    $(MAKE)
    cd ..\shortratemodels
    $(MAKE)
    cd ..\termstructures
    $(MAKE)
    cd ..\utilities
    $(MAKE)
    cd ..\volatilities
    $(MAKE)
    cd ..\volatilitymodels
    $(MAKE)
    cd ..


# Clean up
clean::
    cd calendars
    $(MAKE) clean
    cd ..\cashflows
    $(MAKE) clean
    cd ..\currencies
    $(MAKE) clean
    cd ..\daycounters
    $(MAKE) clean
    cd ..\finitedifferences
    $(MAKE) clean
    cd ..\indexes
    $(MAKE) clean
    cd ..\instruments
    $(MAKE) clean
    cd ..\lattices
    $(MAKE) clean
    cd ..\marketmodels
    $(MAKE) clean
    cd ..\math
    $(MAKE) clean
    cd ..\montecarlo
    $(MAKE) clean
    cd ..\optimization
    $(MAKE) clean
    cd ..\pricers
    $(MAKE) clean
    cd ..\pricingengines
    $(MAKE) clean
    cd ..\processes
    $(MAKE) clean
    cd ..\randomnumbers
    $(MAKE) clean
    cd ..\shortratemodels
    $(MAKE) clean
    cd ..\termstructures
    $(MAKE) clean
    cd ..\utilities
    $(MAKE) clean
    cd ..\volatilities
    $(MAKE) clean
    cd ..\volatilitymodels
    $(MAKE) clean
    cd ..
    if exist *.obj*                    del /q *.obj*
    if exist *.lib                     del /q *.lib
    if exist $(OUTPUT_DIR)\*-bcb-*.lib del /q $(OUTPUT_DIR)\*-bcb-*.lib

