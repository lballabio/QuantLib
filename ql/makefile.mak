
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..
BCC_INCLUDE    = $(MAKEDIR)\..\include
OUTPUT_DIR     = ..\lib\Win32\Borland

# Object files
CORE_OBJS = \
    calendar.obj$(_D) \
    dataformatters.obj$(_D) \
    dataparsers.obj$(_D) \
    date.obj$(_D) \
    discretizedasset.obj$(_D) \
    diffusionprocess.obj$(_D) \
    exercise.obj$(_D) \
    grid.obj$(_D) \
    scheduler.obj$(_D) \
    voltermstructure.obj$(_D)

CALENDAR_LIB        = Calendars$(_D).lib
CALIBRATION_LIB     = CalibrationHelpers$(_D).lib
CASHFLOWS_LIB       = CashFlows$(_D).lib
DAYCOUNTER_LIB      = DayCounters$(_D).lib
FDM_LIB             = FiniteDifferences$(_D).lib
FUNCTIONS           = functions$(_D).lib
INDEXES_LIB         = Indexes$(_D).lib
INSTRUMENTS_LIB     = Instruments$(_D).lib
SHORTRATEMODELS_LIB = ShortRateModels$(_D).lib
LATTICES_LIB        = Lattices$(_D).lib
MATH_LIB            = Math$(_D).lib
MONTECARLO_LIB      = MonteCarlo$(_D).lib
ONEFACTOR_LIB       = OneFactorModels$(_D).lib
OPTIMIZATION_LIB    = Optimization$(_D).lib
PRICER_LIB          = Pricers$(_D).lib
PRICING_ENGINES_LIB = PricingEngines$(_D).lib
RNG_LIB             = RandomNumbers$(_D).lib
TERMSTRUCT_LIB      = TermStructures$(_D).lib
TWOFACTOR_LIB       = TwoFactorModels$(_D).lib
VOLATILITY_LIB      = Volatilities$(_D).lib

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
    $(MONTECARLO_LIB) \
    $(ONEFACTOR_LIB) \
    $(OPTIMIZATION_LIB) \
    $(PRICER_LIB) \
    $(PRICING_ENGINES_LIB) \
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
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
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
$(OUTPUT_DIR)\QuantLib$(_D).lib:: $(CORE_OBJS) SubLibraries
    if exist $(OUTPUT_DIR)\QuantLib$(_D).lib del $(OUTPUT_DIR)\QuantLib$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OUTPUT_DIR)\QuantLib$(_D).lib /a $(QUANTLIB_OBJS)


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
    cd ..\MonteCarlo
    $(MAKE)
    cd ..\Optimization
    $(MAKE)
    cd ..\Pricers
    $(MAKE)
    cd ..\PricingEngines
    $(MAKE)
    cd ..\RandomNumbers
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
    cd ..\ShortRateModels
    $(MAKE) clean
    cd CalibrationHelpers
    $(MAKE) clean
    cd ..\OneFactorModels
    $(MAKE) clean
    cd ..\TwoFactorModels
    $(MAKE) clean
    cd ..\..\Lattices
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
    cd ..\RandomNumbers
    $(MAKE) clean
    cd ..\TermStructures
    $(MAKE) clean
    cd ..\Volatilities
    $(MAKE) clean
    cd ..
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib                   del /q *.lib
    if exist $(OUTPUT_DIR)\*.lib     del /q $(OUTPUT_DIR)\*.lib

