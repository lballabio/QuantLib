
# $Id$



.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..
BCC_INCLUDE    = $(MAKEDIR)\..\include

# Object files
OBJS = \
    capfloor.obj$(_D) \
    forwardvanillaoption.obj$(_D) \
    quantovanillaoption.obj$(_D) \
    simpleswap.obj$(_D) \
    stock.obj$(_D) \
    swap.obj$(_D) \
    swaption.obj$(_D) \
    vanillaoption.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -vi- -q -c -tWM -n$(OUTPUT_DIR) \
                 -w-8026 -w-8027 -w-8012 \
                 -I$(INCLUDE_DIR) \
                 -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif

TLIB_OPTS    = /P128
!ifdef DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_D):
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

