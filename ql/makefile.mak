
# $Id$

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib
OUTPUT_DIR     = ..\lib\Win32\Borland

# Object files
CORE_OBJS        = calendar.obj$(_D) \
                   dataformatters.obj$(_D) \
                   date.obj$(_D) \
                   option.obj$(_D) \
                   scheduler.obj$(_D) \
                   solver1d.obj$(_D)

CALENDAR_LIB     = Calendars\Calendars$(_D).lib
CALIBRATION_LIB  = InterestRateModelling\CalibrationHelpers\CalibrationHelpers$(_D).lib
CASHFLOWS_LIB    = CashFlows\CashFlows$(_D).lib
DAYCOUNTER_LIB   = DayCounters\DayCounters$(_D).lib
FDM_LIB          = FiniteDifferences\FiniteDifferences$(_D).lib
INDEXES_LIB      = Indexes\Indexes$(_D).lib
INSTRUMENTS_LIB  = Instruments\Instruments$(_D).lib
IRMODELING_LIB   = InterestRateModelling\IRModelling$(_D).lib
LATTICES_LIB     = Lattices\Lattices$(_D).lib
MATH_LIB         = Math\Math$(_D).lib
MONTECARLO_LIB   = MonteCarlo\MonteCarlo$(_D).lib
ONEFACTOR_LIB    = InterestRateModelling\OneFactorModels\OneFactorModels$(_D).lib
OPTIMIZATION_LIB = Optimization\Optimization$(_D).lib
PRICER_LIB       = Pricers\Pricers$(_D).lib
RNG_LIB          = RandomNumbers\RandomNumbers$(_D).lib
SOLVER1D_LIB     = Solvers1D\Solvers1D$(_D).lib
TERMSTRUCT_LIB   = TermStructures\TermStructures$(_D).lib

QUANTLIB_OBJS    = \
                   $(CALENDAR_LIB) \
                   $(CALIBRATION_LIB) \
                   $(CASHFLOWS_LIB) \
                   $(CORE_OBJS) \
                   $(DAYCOUNTER_LIB) \
                   $(FDM_LIB) \
                   $(INDEXES_LIB) \
                   $(INSTRUMENTS_LIB) \
                   $(IRMODELING_LIB) \
                   $(LATTICES_LIB) \
                   $(MATH_LIB) \
                   $(MONTECARLO_LIB) \
                   $(ONEFACTOR_LIB) \
                   $(OPTIMIZATION_LIB) \
                   $(PRICER_LIB) \
                   $(RNG_LIB) \
                   $(SOLVER1D_LIB) \
                   $(TERMSTRUCT_LIB)

# Tools to be used
CC        = bcc32
TLIB      = tlib
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif

# Options
CC_OPTS        = -vi- -q -c -tWM \
    -w-8026 -w-8027 -w-8012 \
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DSAFE_CHECKS
!endif

TLIB_OPTS    = /P128
!ifdef DEBUG
TLIB_OPTS    = /P256
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj_d:
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
    cd ..\Indexes
    $(MAKE)
    cd ..\Instruments
    $(MAKE)
    cd ..\InterestRateModelling
    $(MAKE)
    cd CalibrationHelpers
    $(MAKE)
    cd ..\OneFactorModels
    $(MAKE)
    cd ..\..\Lattices
    $(MAKE)
    cd ..\Math
    $(MAKE)
    cd ..\MonteCarlo
    $(MAKE)
    cd ..\Optimization
    $(MAKE)
    cd ..\Pricers
    $(MAKE)
    cd ..\RandomNumbers
    $(MAKE)
    cd ..\Solvers1D
    $(MAKE)
    cd ..\TermStructures
    $(MAKE)
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
    cd ..\Indexes
    $(MAKE) clean
    cd ..\Instruments
    $(MAKE) clean
    cd ..\InterestRateModelling
    $(MAKE) clean
    cd CalibrationHelpers
    $(MAKE) clean
    cd ..\OneFactorModels
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
    cd ..\RandomNumbers
    $(MAKE) clean
    cd ..\Solvers1D
    $(MAKE) clean
    cd ..\TermStructures
    $(MAKE) clean
    cd ..
    if exist *.obj      del /q *.obj
    if exist *.obj_d    del /q *.obj
    if exist $(OUTPUT_DIR)\*.lib  del /q $(OUTPUT_DIR)\*.lib

