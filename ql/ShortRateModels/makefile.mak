
.autodepend
.silent

MAKE = $(MAKE)

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
    "calibrationhelper.obj$(_mt)$(_D)" \
    "model.obj$(_mt)$(_D)" \
    "onefactormodel.obj$(_mt)$(_D)" \
    "twofactormodel.obj$(_mt)$(_D)" \
    "CalibrationHelpers\CalibrationHelpers$(_mt)$(_D).lib" \
    "OneFactorModels\OneFactorModels$(_mt)$(_D).lib" \
    "TwoFactorModels\TwoFactorModels$(_mt)$(_D).lib"

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

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
ShortRateModels$(_mt)$(_D).lib:: SubLibraries $(OBJS)
    if exist ShortRateModels$(_mt)$(_D).lib     del ShortRateModels$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "ShortRateModels$(_mt)$(_D).lib" /a $(OBJS)

SubLibraries:
    cd CalibrationHelpers
    $(MAKE)
    cd ..\OneFactorModels
    $(MAKE)
    cd ..\TwoFactorModels
    $(MAKE)
    cd ..

# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_mt)$(_D)""    del /q *.obj
    if exist *.lib         del /q *.lib
    cd CalibrationHelpers
    $(MAKE) clean
    cd ..\OneFactorModels
    $(MAKE) clean
    cd ..\TwoFactorModels
    $(MAKE) clean
    cd ..
