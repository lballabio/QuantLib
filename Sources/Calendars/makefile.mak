
# $Source$

#
# makefile for QuantLib calendar library under Borland C++
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
OBJS = westerncalendar.obj$(_D) \
       frankfurt.obj$(_D) \
       helsinki.obj$(_D) \
       london.obj$(_D) \
       milan.obj$(_D) \
       newyork.obj$(_D) \
       target.obj$(_D) \
       wellington.obj$(_D) \
       zurich.obj$(_D)

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
Calendars$(_D).lib:: Message $(OBJS)
    if exist Calendars$(_D).lib     del Calendars$(_D).lib
    $(TLIB) $(TLIB_OPTS) Calendars$(_D).lib /a $(OBJS)

Message:
    echo Building calendar library...

westerncalendar.obj$(_D):
frankfurt.obj$(_D):
helsinki.obj$(_D):
london.obj$(_D):
milan.obj$(_D):
newyork.obj$(_D):
target.obj$(_D):
wellington.obj$(_D):
zurich.obj$(_D):

# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

