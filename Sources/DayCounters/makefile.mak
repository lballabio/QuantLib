

# $Id$
# $Source$
# $Log$
# Revision 1.6  2001/08/29 18:12:59  nando
# added 2 more ActualActual day count convention
# not finished yet
#
# Revision 1.5  2001/05/24 12:52:02  nando
# smoothing #include xx.hpp
#
#
# makefile for QuantLib day counter library under Borland C++
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
OBJS = \
    actualactual.obj$(_D) \
    actualactualeuro.obj$(_D) \
    actualactualhistorical.obj$(_D) \
    thirty360.obj$(_D) \
    thirty360italian.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -q -c -tWM -n$(OUTPUT_DIR) \
    -w-8026 -w-8027 -w-8012 \
    -I$(INCLUDE_DIR) \
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
DayCounters$(_D).lib:: $(OBJS)
    if exist DayCounters$(_D).lib     del DayCounters$(_D).lib
    $(TLIB) $(TLIB_OPTS) DayCounters$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

