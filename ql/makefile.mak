
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..
OUTPUT_DIR     = ..\lib\Win32\Borland

# Object files
CORE_OBJS = \
    calendar.obj$(_D) \
    dataformatters.obj$(_D) \
    dataparsers.obj$(_D) \
    date.obj$(_D) \
    diffusionprocess.obj$(_D) \
    discretizedasset.obj$(_D) \
    errors.obj$(_D) \
    exercise.obj$(_D) \
    grid.obj$(_D) \
    scheduler.obj$(_D) \
    voltermstructure.obj$(_D)

CALENDAR_LIB         = Calendars\Calendars$(_D).lib
CASHFLOWS_LIB        = CashFlows\CashFlows$(_D).lib
DAYCOUNTER_LIB       = DayCounters\DayCounters$(_D).lib
FDM_LIB              = FiniteDifferences\FiniteDifferences$(_D).lib
FUNCTIONS            = functions\functions$(_D).lib
INDEXES_LIB          = Indexes\Indexes$(_D).lib
INSTRUMENTS_LIB      = Instruments\Instruments$(_D).lib
LATTICES_LIB         = Lattices\Lattices$(_D).lib
MATH_LIB             = Math\Math$(_D).lib
OPTIMIZATION_LIB     = Optimization\Optimization$(_D).lib
PRICER_LIB           = Pricers\Pricers$(_D).lib
ASIAN_ENGINES_LIB    = PricingEngines\Asian\AsianEngines$(_D).lib
BARRIER_ENGINES_LIB  = PricingEngines\Barrier\BarrierEngines$(_D).lib
BASKET_ENGINES_LIB   = PricingEngines\Basket\BasketEngines$(_D).lib
CAPFLOOR_ENGINES_LIB = PricingEngines\CapFloor\CapFloorEngines$(_D).lib
SWAPTION_ENGINES_LIB = PricingEngines\Swaption\SwaptionEngines$(_D).lib
VANILLA_ENGINES_LIB  = PricingEngines\Vanilla\VanillaEngines$(_D).lib
RNG_LIB              = RandomNumbers\RandomNumbers$(_D).lib
SHORTRATEMODELS_LIB  = ShortRateModels\ShortRateModels$(_D).lib
CALIBRATION_LIB      = \
                ShortRateModels\CalibrationHelpers\CalibrationHelpers$(_D).lib
ONEFACTOR_LIB        = \
                ShortRateModels\OneFactorModels\OneFactorModels$(_D).lib
TWOFACTOR_LIB        = \
                ShortRateModels\TwoFactorModels\TwoFactorModels$(_D).lib
TERMSTRUCT_LIB       = TermStructures\TermStructures$(_D).lib
VOLATILITY_LIB       = Volatilities\Volatilities$(_D).lib

QUANTLIB_OBJS = \
    $(CALENDAR_LIB) \
    $(CALIBRATION_LIB) \
    $(CASHFLOWS_LIB) \
    $(CORE_OBJS) \
    $(DAYCOUNTER_LIB) \
    $(FDM_LIB) \
    $(FUNCTIONS) \
    $(INDEXES_LIB) \
    $(INSTRUMENTS_LIB) \
    $(SHORTRATEMODELS_LIB) \
    $(LATTICES_LIB) \
    $(MATH_LIB) \
    $(ONEFACTOR_LIB) \
    $(OPTIMIZATION_LIB) \
    $(PRICER_LIB) \
    $(ASIAN_ENGINES_LIB) \
    $(BARRIER_ENGINES_LIB) \
    $(BASKET_ENGINES_LIB) \
    $(CAPFLOOR_ENGINES_LIB) \
    $(SWAPTION_ENGINES_LIB) \
    $(VANILLA_ENGINES_LIB) \
    $(RNG_LIB) \
    $(TERMSTRUCT_LIB) \
    $(TWOFACTOR_LIB) \
    $(VOLATILITY_LIB)

# Tools to be used
CC        = bcc32
TLIB      = tlib
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif
!ifdef SAFE
    MAKE = $(MAKE) -DSAFE
!endif



# Options
CC_OPTS = -vi- -q -c -tWM \
    -I$(INCLUDE_DIR)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

TLIB_OPTS    = /P512
!ifdef DEBUG
TLIB_OPTS    = /P1024
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# QuantLib library
$(OUTPUT_DIR)\QuantLib$(_D).lib:: $(OUTPUT_DIR) $(CORE_OBJS) SubLibraries
    if exist $(OUTPUT_DIR)\QuantLib$(_D).lib del $(OUTPUT_DIR)\QuantLib$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OUTPUT_DIR)\QuantLib$(_D).lib /a $(QUANTLIB_OBJS)

$(OUTPUT_DIR):
    if not exist ..\lib md ..\lib
    if not exist ..\lib\Win32 md ..\lib\Win32
    if not exist ..\lib\Win32\Borland md ..\lib\Win32\Borland

SubLibraries:
    cd Calendars
    $(MAKE)
    cd ..\CashFlows
    $(MAKE)
    cd ..\DayCounters
    $(MAKE)
    cd ..\FiniteDifferences
    $(MAKE)
    cd ..\functions
    $(MAKE)
    cd ..\Indexes
    $(MAKE)
    cd ..\Instruments
    $(MAKE)
    cd ..\Math
    $(MAKE)
    cd ..\Optimization
    $(MAKE)
    cd ..\Pricers
    $(MAKE)
    cd ..\PricingEngines
    cd Asian
    $(MAKE)
    cd ..\Barrier
    $(MAKE)
    cd ..\Basket
    $(MAKE)
    cd ..\CapFloor
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
    cd ..\DayCounters
    $(MAKE) clean
    cd ..\FiniteDifferences
    $(MAKE) clean
    cd ..\functions
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
    cd Asian
    $(MAKE) clean
    cd ..\Barrier
    $(MAKE) clean
    cd ..\Basket
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
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib                   del /q *.lib
    if exist $(OUTPUT_DIR)\*.lib     del /q $(OUTPUT_DIR)\*.lib

