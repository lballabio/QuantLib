
# $Id$
# $Source$
# $Log$
# Revision 1.5  2001/05/24 12:52:02  nando
# smoothing #include xx.hpp
#
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
Calendars$(_D).lib:: $(OBJS)
    if exist Calendars$(_D).lib     del Calendars$(_D).lib
    $(TLIB) $(TLIB_OPTS) Calendars$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

