
# $Id$
# $Source$
#
# makefile for QuantLib static library under Borland C++
#

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\Include
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib
OUTPUT_DIR     = ..\lib\Win32\Borland

# Object files
CORE_OBJS        = calendar.obj$(_D) \
                   dataformatters.obj$(_D) \
                   date.obj$(_D) \
                   solver1d.obj$(_D)

CALENDAR_LIB     = Calendars\Calendars$(_D).lib
DAYCOUNTER_LIB   = DayCounters\DayCounters$(_D).lib
FDM_LIB          = FiniteDifferences\FiniteDifferences$(_D).lib
MATH_LIB         = Math\Math$(_D).lib
MONTECARLO_LIB   = MonteCarlo\MonteCarlo$(_D).lib
PRICER_LIB       = Pricers\Pricers$(_D).lib
SOLVER1D_LIB     = Solvers1D\Solvers1D$(_D).lib
TERMSTRUCT_LIB   = TermStructures\TermStructures$(_D).lib

QUANTLIB_OBJS    = $(CORE_OBJS) \
                   $(CALENDAR_LIB) \
                   $(DAYCOUNTER_LIB) \
                   $(FDM_LIB) \
                   $(MATH_LIB) \
                   $(MONTECARLO_LIB) \
                   $(PRICER_LIB) \
                   $(SOLVER1D_LIB) \
                   $(TERMSTRUCT_LIB)

# Tools to be used
CC        = bcc32
TLIB      = tlib
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif

# Options
CC_OPTS        = -q -c -tWM \
    -w-8026 -w-8027 -w-8012 \
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DSAFE_CHECKS
!endif

TLIB_OPTS    = /P32
!ifdef DEBUG
TLIB_OPTS    = /P128
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
    cd ..\DayCounters
    $(MAKE)
    cd ..\FiniteDifferences
    $(MAKE)
    cd ..\Math
    $(MAKE)
    cd ..\MonteCarlo
    $(MAKE)
    cd ..\Pricers
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
    cd ..\DayCounters
    $(MAKE) clean
    cd ..\FiniteDifferences
    $(MAKE) clean
    cd ..\Math
    $(MAKE) clean
    cd ..\MonteCarlo
    $(MAKE) clean
    cd ..\Pricers
    $(MAKE) clean
    cd ..\Solvers1D
    $(MAKE) clean
    cd ..\TermStructures
    $(MAKE) clean
    cd ..
    if exist *.obj      del /q *.obj
    if exist *.obj_d    del /q *.obj
    if exist $(OUTPUT_DIR)\*.lib  del /q $(OUTPUT_DIR)\*.lib

