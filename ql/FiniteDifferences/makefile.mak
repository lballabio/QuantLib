
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..

# Object files
OBJS = \
    bsmoperator.obj$(_D) \
    boundarycondition.obj$(_D) \
    onefactoroperator.obj$(_D) \
    tridiagonaloperator.obj$(_D) \
    valueatcenter.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib



# Options
CC_OPTS        = -vi- -q -c -tWM \
    -I$(INCLUDE_DIR)

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
FiniteDifferences$(_D).lib:: $(OBJS)
    if exist FiniteDifferences$(_D).lib     del FiniteDifferences$(_D).lib
    $(TLIB) $(TLIB_OPTS) FiniteDifferences$(_D).lib /a $(OBJS)





# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib         del /q *.lib
