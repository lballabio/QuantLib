
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
BOOST_INCLUDE_DIR   = "$(BOOST_DIR)"
INCLUDE_DIR    = ..\..
BCC_INCLUDE    = $(MAKEDIR)\..\include

# Object files
OBJS = \
    beta.obj$(_D) \
    bivariatenormaldistribution.obj$(_D) \
    chisquaredistribution.obj$(_D) \
    choleskydecomposition.obj$(_D) \
    discrepancystatistics.obj$(_D) \
    errorfunction.obj$(_D) \
    factorial.obj$(_D) \
    gammadistribution.obj$(_D) \
    generalstatistics.obj$(_D) \
    incompletegamma.obj$(_D) \
    incrementalstatistics.obj$(_D) \
    normaldistribution.obj$(_D) \
    primenumbers.obj$(_D) \
    pseudosqrt.obj$(_D) \
    svd.obj$(_D) \
    symmetricschurdecomposition.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib


# Options
CC_OPTS        = -vi- -q -c -tWM \
    -I$(BOOST_INCLUDE_DIR) \
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE)

!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
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
Math$(_D).lib:: $(OBJS)
    if exist Math$(_D).lib     del Math$(_D).lib
    $(TLIB) $(TLIB_OPTS) Math$(_D).lib /a $(OBJS)





# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib         del /q *.lib
