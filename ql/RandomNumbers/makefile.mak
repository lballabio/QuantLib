
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR      = ..\..
BCC_INCLUDE      = $(MAKEDIR)\..\include

# Object files
OBJS = \
    primitivepolynomials.obj$(_D) \
    haltonrsg.obj$(_D) \
    knuthuniformrng.obj$(_D) \
    lecuyeruniformrng.obj$(_D) \
    mt19937uniformrng.obj$(_D) \
    sobolrsg.obj$(_D)

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

TLIB_OPTS    = /P256
!ifdef DEBUG
TLIB_OPTS    = /P256
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<
.c.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
RandomNumbers$(_D).lib:: $(OBJS)
    if exist RandomNumbers$(_D).lib     del RandomNumbers$(_D).lib
    $(TLIB) $(TLIB_OPTS) RandomNumbers$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib   del /q *.lib

