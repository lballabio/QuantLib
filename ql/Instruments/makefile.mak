
.autodepend
.silent

!ifdef _DEBUG
!ifndef _RTLDLL
    _D = -sd
!else
    _D = -d
!endif
!else
!ifndef _RTLDLL
    _D = -s
!endif
!endif

!ifdef __MT__
    _mt = -mt
!endif

# Directories
INCLUDE_DIR    = ..\..


# Object files
OBJS = \
    "asianoption.obj$(_mt)$(_D)" \
    "barrieroption.obj$(_mt)$(_D)" \
    "basketoption.obj$(_mt)$(_D)" \
    "bond.obj$(_mt)$(_D)" \
    "capfloor.obj$(_mt)$(_D)" \
    "cliquetoption.obj$(_mt)$(_D)" \
    "dividendvanillaoption.obj$(_mt)$(_D)" \
    "europeanoption.obj$(_mt)$(_D)" \
    "fixedcouponbond.obj$(_mt)$(_D)" \
    "floatingratebond.obj$(_mt)$(_D)" \
    "forwardvanillaoption.obj$(_mt)$(_D)" \
    "multiassetoption.obj$(_mt)$(_D)" \
    "oneassetoption.obj$(_mt)$(_D)" \
    "oneassetstrikedoption.obj$(_mt)$(_D)" \
    "quantoforwardvanillaoption.obj$(_mt)$(_D)" \
    "quantovanillaoption.obj$(_mt)$(_D)" \
    "simpleswap.obj$(_mt)$(_D)" \
    "stock.obj$(_mt)$(_D)" \
    "swap.obj$(_mt)$(_D)" \
    "swaption.obj$(_mt)$(_D)" \
    "vanillaoption.obj$(_mt)$(_D)" \
    "zerocouponbond.obj$(_mt)$(_D)"

# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS = -vi- -q -c -I$(INCLUDE_DIR)

!ifdef _DEBUG
    CC_OPTS = $(CC_OPTS) -v -D_DEBUG
!else
    CC_OPTS = $(CC_OPTS) -O2 -DNDEBUG
!endif

!ifdef _RTLDLL
    CC_OPTS = $(CC_OPTS) -D_RTLDLL
!endif

!ifdef __MT__
    CC_OPTS = $(CC_OPTS) -tWM
!endif

!ifdef SAFE
    CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

TLIB_OPTS    = /P128
!ifdef _DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
Instruments$(_mt)$(_D).lib:: $(OBJS)
    if exist Instruments$(_mt)$(_D).lib     del Instruments$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "Instruments$(_mt)$(_D).lib" /a $(OBJS)


# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
