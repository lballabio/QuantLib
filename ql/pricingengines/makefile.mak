
.autodepend
#.silent

MAKE = $(MAKE) -fmakefile.mak

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
    "americanpayoffatexpiry.obj$(_mt)$(_D)" \
    "americanpayoffathit.obj$(_mt)$(_D)" \
    "blackformula.obj$(_mt)$(_D)" \
    "greeks.obj$(_mt)$(_D)" \
    "asian\AsianEngines$(_mt)$(_D).lib" \
    "barrier\BarrierEngines$(_mt)$(_D).lib" \
    "basket\BasketEngines$(_mt)$(_D).lib" \
    "capfloor\CapFloorEngines$(_mt)$(_D).lib" \
    "cliquet\CliquetEngines$(_mt)$(_D).lib" \
    "hybrid\HybridEngines$(_mt)$(_D).lib" \
    "lookback\LookbackEngines$(_mt)$(_D).lib" \
    "swaption\SwaptionEngines$(_mt)$(_D).lib" \
    "vanilla\VanillaEngines$(_mt)$(_D).lib"


# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -vi- -q -c -I$(INCLUDE_DIR) -w-8070

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

# MAKE Options
!ifdef __MT__
    MAKE = $(MAKE) -D__MT__
!endif
!ifdef _RTLDLL
    MAKE = $(MAKE) -D_RTLDLL
!endif
!ifdef _DEBUG
    MAKE = $(MAKE) -D_DEBUG
!endif
!ifdef SAFE
    MAKE = $(MAKE) -DSAFE
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
PricingEngines$(_mt)$(_D).lib:: SubLibraries $(OBJS)
    if exist PricingEngines$(_mt)$(_D).lib    del PricingEngines$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "PricingEngines$(_mt)$(_D).lib" /a $(OBJS)

SubLibraries:
    cd asian
    $(MAKE)
    cd ..\barrier
    $(MAKE)
    cd ..\basket
    $(MAKE)
    cd ..\capfloor
    $(MAKE)
    cd ..\cliquet
    $(MAKE)
    cd ..\hybrid
    $(MAKE)
    cd ..\lookback
    $(MAKE)
    cd ..\swaption
    $(MAKE)
    cd ..\vanilla
    $(MAKE)
    cd ..


# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
    cd asian
    $(MAKE) clean
    cd ..\barrier
    $(MAKE) clean
    cd ..\basket
    $(MAKE) clean
    cd ..\capfloor
    $(MAKE) clean
    cd ..\cliquet
    $(MAKE) clean
    cd ..\hybrid
    $(MAKE) clean
    cd ..\lookback
    $(MAKE) clean
    cd ..\swaption
    $(MAKE) clean
    cd ..\vanilla
    $(MAKE) clean
    cd ..
