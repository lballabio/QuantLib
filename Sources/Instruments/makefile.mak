
# $Id$
# $Source$
# $Log$
# Revision 1.2  2001/05/31 13:17:36  lballabio
# Added SimpleSwap
#
# Revision 1.1  2001/05/31 08:56:40  lballabio
# Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
#
#
# makefile for QuantLib instrument library under Borland C++
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
OBJS = swap.obj$(_D) \
       simpleswap.obj$(_D)

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
Instruments$(_D).lib:: $(OBJS)
    if exist Instruments$(_D).lib     del Instruments$(_D).lib
    $(TLIB) $(TLIB_OPTS) Instruments$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

