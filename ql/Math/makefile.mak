
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..
BCC_INCLUDE    = $(MAKEDIR)\..\include
SRCDIR         = "."
OBJDIR         = "..\..\build\Borland"

# Object files
OBJS = \
    $(OBJDIR)\chisquaredistribution.obj$(_D) \
    $(OBJDIR)\discrepancystatistics.obj$(_D) \
    $(OBJDIR)\errorfunction.obj$(_D) \
    $(OBJDIR)\gammadistribution.obj$(_D) \
    $(OBJDIR)\generalstatistics.obj$(_D) \
    $(OBJDIR)\incrementalstatistics.obj$(_D) \
    $(OBJDIR)\matrix.obj$(_D) \
    $(OBJDIR)\multivariateaccumulator.obj$(_D) \
    $(OBJDIR)\normaldistribution.obj$(_D) \
    $(OBJDIR)\primenumbers.obj$(_D) \
    $(OBJDIR)\svd.obj$(_D) \
    $(OBJDIR)\symmetricschurdecomposition.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib


# Options
CC_OPTS        = -vi- -q -c -tWM \
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE) \
    -n$(OBJDIR)

!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

TLIB_OPTS    = /P128
!ifdef DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    $(CC) $(CC_OPTS) $<
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
$(OBJDIR)\Math$(_D).lib:: $(OBJDIR) $(OBJS)
    if exist $(OBJDIR)\Math$(_D).lib     del $(OBJDIR)\Math$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OBJDIR)\Math$(_D).lib /a $(OBJS)

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

# Clean up
clean::
    if exist $(OBJDIR)\*.obj         del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.obj$(_D)    del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib         del /q $(OBJDIR)\*.lib
