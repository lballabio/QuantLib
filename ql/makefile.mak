
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
SRCDIR         = "."
OBJDIR         = "..\build\Borland"

# Object files
CORE_OBJS = \
    $(OBJDIR)\calendar.obj$(_D) \
    $(OBJDIR)\dataformatters.obj$(_D) \
    $(OBJDIR)\dataparsers.obj$(_D) \
    $(OBJDIR)\date.obj$(_D) \
    $(OBJDIR)\discretizedasset.obj$(_D) \
    $(OBJDIR)\diffusionprocess.obj$(_D) \
    $(OBJDIR)\exercise.obj$(_D) \
    $(OBJDIR)\grid.obj$(_D) \
    $(OBJDIR)\scheduler.obj$(_D) \
    $(OBJDIR)\voltermstructure.obj$(_D)

CALENDAR_LIB        = $(OBJDIR)\Calendars$(_D).lib
CALIBRATION_LIB     = $(OBJDIR)\CalibrationHelpers$(_D).lib
CASHFLOWS_LIB       = $(OBJDIR)\CashFlows$(_D).lib
DAYCOUNTER_LIB      = $(OBJDIR)\DayCounters$(_D).lib
FDM_LIB             = $(OBJDIR)\FiniteDifferences$(_D).lib
FUNCTIONS           = $(OBJDIR)\functions$(_D).lib
INDEXES_LIB         = $(OBJDIR)\Indexes$(_D).lib
INSTRUMENTS_LIB     = $(OBJDIR)\Instruments$(_D).lib
SHORTRATEMODELS_LIB = $(OBJDIR)\ShortRateModels$(_D).lib
LATTICES_LIB        = $(OBJDIR)\Lattices$(_D).lib
MATH_LIB            = $(OBJDIR)\Math$(_D).lib
MONTECARLO_LIB      = $(OBJDIR)\MonteCarlo$(_D).lib
ONEFACTOR_LIB       = $(OBJDIR)\OneFactorModels$(_D).lib
OPTIMIZATION_LIB    = $(OBJDIR)\Optimization$(_D).lib
PRICER_LIB          = $(OBJDIR)\Pricers$(_D).lib
PRICING_ENGINES_LIB = $(OBJDIR)\PricingEngines$(_D).lib
RNG_LIB             = $(OBJDIR)\RandomNumbers$(_D).lib
TERMSTRUCT_LIB      = $(OBJDIR)\TermStructures$(_D).lib
TWOFACTOR_LIB       = $(OBJDIR)\TwoFactorModels$(_D).lib

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
    $(TWOFACTOR_LIB)

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
    -I$(BCC_INCLUDE) \
    -n$(OBJDIR)
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
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    $(CC) $(CC_OPTS) $<
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# QuantLib library
$(OUTPUT_DIR)\QuantLib$(_D).lib:: $(OBJDIR) $(CORE_OBJS) SubLibraries
    if exist $(OUTPUT_DIR)\QuantLib$(_D).lib del $(OUTPUT_DIR)\QuantLib$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OUTPUT_DIR)\QuantLib$(_D).lib /a $(QUANTLIB_OBJS)

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

SubLibraries:
    cd Calendars
    $(MAKE)
    cd ..\CashFlows
    $(MAKE)
    cd ..\DayCounters
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
    cd ..\FiniteDifferences
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
    cd ..
    if exist $(OBJDIR)\*.obj         del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.obj$(_D)    del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib         del /q $(OBJDIR)\*.lib
    if exist $(OUTPUT_DIR)\*.lib     del /q $(OUTPUT_DIR)\*.lib

