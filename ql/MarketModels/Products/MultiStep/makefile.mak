
.autodepend
#.silent

# Debug version
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
    "callspecifiedmultiproduct.obj$(_mt)$(_D)" \
    "cashrebate.obj$(_mt)$(_D)" \
    "exerciseadapter.obj$(_mt)$(_D)" \
    "multistepcaplets.obj$(_mt)$(_D)" \
    "multistepcoinitialswaps.obj$(_mt)$(_D)" \
    "multistepcoterminalswaps.obj$(_mt)$(_D)" \
    "multistepforwards.obj$(_mt)$(_D)" \
    "multistepnothing.obj$(_mt)$(_D)" \
    "multistepswap.obj$(_mt)$(_D)"

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
MultiStep$(_mt)$(_D).lib:: $(OBJS)
    if exist MultiStep$(_mt)$(_D).lib     del MultiStep$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "MultiStep$(_mt)$(_D).lib" /a $(OBJS)


# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
