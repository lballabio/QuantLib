
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
    "accountingengine.obj$(_mt)$(_D)" \
    "curvestate.obj$(_mt)$(_D)" \
    "driftcalculator.obj$(_mt)$(_D)" \
    "evolutiondescription.obj$(_mt)$(_D)" \
    "lsdatacollector.obj$(_mt)$(_D)" \
    "marketmodeldiscounter.obj$(_mt)$(_D)" \
    "swapbasissystem.obj$(_mt)$(_D)" \
    "swapforwardconversionmatrix.obj$(_mt)$(_D)" \
    "utilities.obj$(_mt)$(_D)" \
    "browniangenerators\BrownianGenerators$(_mt)$(_D).lib" \
    "evolvers\Evolvers$(_mt)$(_D).lib" \
    "exercisestrategies\ExerciseStrategies$(_mt)$(_D).lib" \
    "exercisevalues\ExerciseValues$(_mt)$(_D).lib" \
    "models\Models$(_mt)$(_D).lib" \
    "products\Products$(_mt)$(_D).lib"

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
MarketModels$(_mt)$(_D).lib:: SubLibraries $(OBJS)
    if exist MarketModels$(_mt)$(_D).lib     del MarketModels$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "MarketModels$(_mt)$(_D).lib" /a $(OBJS)

SubLibraries:
    cd browniangenerators
    $(MAKE)
    cd ..\evolvers
    $(MAKE)
    cd ..\exercisestrategies
    $(MAKE)
    cd ..\exercisevalues
    $(MAKE)
    cd ..\models
    $(MAKE)
    cd ..\products
    $(MAKE)
    cd ..

# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_mt)$(_D)""    del /q *.obj
    if exist *.lib         del /q *.lib
    cd browniangenerators
    $(MAKE) clean
    cd ..\evolvers
    $(MAKE) clean
    cd ..\exercisestrategies
    $(MAKE) clean
    cd ..\exercisevalues
    $(MAKE) clean
    cd ..\models
    $(MAKE) clean
    cd ..\products
    $(MAKE) clean
    cd ..
