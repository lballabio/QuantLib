
# $Source$

#
# makefile for QuantLib 1D solver library under Borland C++
#

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..\Include
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib

# Object files
OBJS = bisection.obj$(_D) \
       brent.obj$(_D) \
       falseposition.obj$(_D) \
       newton.obj$(_D) \
       newtonsafe.obj$(_D) \
       ridder.obj$(_D) \
       secant.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib

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
.cpp.obj_d:
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
Solvers1D$(_D).lib:: $(OBJS)
    if exist Solvers1D$(_D).lib     del Solvers1D$(_D).lib
    $(TLIB) $(TLIB_OPTS) Solvers1D$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

