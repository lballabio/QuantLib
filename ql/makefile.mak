
.autodepend
.silent

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
    "basicdataformatters.obj$(_mt)$(_D)" \
    "calendar.obj$(_mt)$(_D)" \
    "currency.obj$(_mt)$(_D)" \
    "dataformatters.obj$(_mt)$(_D)" \
    "dataparsers.obj$(_mt)$(_D)" \
    "date.obj$(_mt)$(_D)" \
    "discretizedasset.obj$(_mt)$(_D)" \
    "errors.obj$(_mt)$(_D)" \
    "exchangerate.obj$(_mt)$(_D)" \
    "exercise.obj$(_mt)$(_D)" \
    "grid.obj$(_mt)$(_D)" \
    "interestrate.obj$(_mt)$(_D)" \
    "money.obj$(_mt)$(_D)" \
    "schedule.obj$(_mt)$(_D)" \
    "stochasticprocess.obj$(_mt)$(_D)" \
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
ASIAN_ENGINES_LIB    = "PricingEngines\Asian\AsianEngines$(_mt)$(_D).lib"
BARRIER_ENGINES_LIB  = "PricingEngines\Barrier\BarrierEngines$(_mt)$(_D).lib"
BASKET_ENGINES_LIB   = "PricingEngines\Basket\BasketEngines$(_mt)$(_D).lib"
CAPFLOOR_ENGINES_LIB = "PricingEngines\CapFloor\CapFloorEngines$(_mt)$(_D).lib"
CLIQUET_ENGINES_LIB  = "PricingEngines\Cliquet\CliquetEngines$(_mt)$(_D).lib"
SWAPTION_ENGINES_LIB = "PricingEngines\Swaption\SwaptionEngines$(_mt)$(_D).lib"
VANILLA_ENGINES_LIB  = "PricingEngines\Vanilla\VanillaEngines$(_mt)$(_D).lib"
RNG_LIB              = "RandomNumbers\RandomNumbers$(_mt)$(_D).lib"
SHORTRATEMODELS_LIB  = "ShortRateModels\ShortRateModels$(_mt)$(_D).lib"
CALIBRATION_LIB      = \
         "ShortRateModels\CalibrationHelpers\CalibrationHelpers$(_mt)$(_D).lib"
ONEFACTOR_LIB        = \
         "ShortRateModels\OneFactorModels\OneFactorModels$(_mt)$(_D).lib"
TWOFACTOR_LIB        = \
         "ShortRateModels\TwoFactorModels\TwoFactorModels$(_mt)$(_D).lib"
TERMSTRUCT_LIB       = "TermStructures\TermStructures$(_mt)$(_D).lib"
VOLATILITY_LIB       = "Volatilities\Volatilities$(_mt)$(_D).lib"

QUANTLIB_OBJS = \
    $(CALENDAR_LIB) \
    $(CALIBRATION_LIB) \
    $(CASHFLOWS_LIB) \
    $(CURRENCIES_LIB) \
    $(CORE_OBJS) \
    $(DAYCOUNTER_LIB) \
    $(FDM_LIB) \
    $(INDEXES_LIB) \
    $(INSTRUMENTS_LIB) \
    $(SHORTRATEMODELS_LIB) \
    $(LATTICES_LIB) \
    $(MATH_LIB) \
    $(MONTECARLO_LIB) \
    $(ONEFACTOR_LIB) \
    $(OPTIMIZATION_LIB) \
    $(PRICER_LIB) \
    $(PRICING_ENGINES_LIB) \
    $(ASIAN_ENGINES_LIB) \
    $(BARRIER_ENGINES_LIB) \
    $(BASKET_ENGINES_LIB) \
    $(CAPFLOOR_ENGINES_LIB) \
    $(CLIQUET_ENGINES_LIB) \
    $(SWAPTION_ENGINES_LIB) \
    $(VANILLA_ENGINES_LIB) \
    $(RNG_LIB) \
    $(TERMSTRUCT_LIB) \
    $(TWOFACTOR_LIB) \
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
CC_OPTS = -vi- -q -c -I$(INCLUDE_DIR)

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
$(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-0_3_8.lib:: $(OUTPUT_DIR) $(CORE_OBJS) \
                                                                SubLibraries
    if exist $(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-0_3_8.lib \
         del $(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-0_3_8.lib
    $(TLIB) $(TLIB_OPTS) "$(OUTPUT_DIR)\QuantLib-bcb$(_mt)$(_D)-0_3_8.lib" \
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
    cd Asian
    $(MAKE)
    cd ..\Barrier
    $(MAKE)
    cd ..\Basket
    $(MAKE)
    cd ..\CapFloor
    $(MAKE)
    cd ..\Cliquet
    $(MAKE)
    cd ..\Swaption
    $(MAKE)
    cd ..\Vanilla
    $(MAKE)
    cd ..\..\RandomNumbers
    $(MAKE)
    cd ..\Lattices
    $(MAKE)
    cd ..\TermStructures
    $(MAKE)
    cd ..\Volatilities
    $(MAKE)
    cd ..\ShortRateModels
    $(MAKE)
    cd CalibrationHelpers
    $(MAKE)
    cd ..\OneFactorModels
    $(MAKE)
    cd ..\TwoFactorModels
    $(MAKE)
    cd ..
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
    cd ..\Math
    $(MAKE) clean
    cd ..\Optimization
    $(MAKE) clean
    cd ..\Pricers
    $(MAKE) clean
    cd ..\PricingEngines
    $(MAKE) clean
    cd Asian
    $(MAKE) clean
    cd ..\Barrier
    $(MAKE) clean
    cd ..\Basket
    $(MAKE) clean
    cd ..\Cliquet
    $(MAKE) clean
    cd ..\CapFloor
    $(MAKE) clean
    cd ..\Swaption
    $(MAKE) clean
    cd ..\Vanilla
    $(MAKE) clean
    cd ..\..\RandomNumbers
    $(MAKE) clean
    cd ..\Lattices
    $(MAKE) clean
    cd ..\TermStructures
    $(MAKE) clean
    cd ..\Volatilities
    $(MAKE) clean
    cd ..\ShortRateModels
    $(MAKE) clean
    cd CalibrationHelpers
    $(MAKE) clean
    cd ..\OneFactorModels
    $(MAKE) clean
    cd ..\TwoFactorModels
    $(MAKE) clean
    cd ..
    cd ..
    if exist *.obj*                    del /q *.obj*
    if exist *.lib                     del /q *.lib
    if exist $(OUTPUT_DIR)\*-bcb-*.lib del /q $(OUTPUT_DIR)\*-bcb-*.lib

