
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR      = ..\..
BCC_INCLUDE      = $(MAKEDIR)\..\include
SRCDIR         = "."
OBJDIR         = "..\..\build\Borland"

# Object files
OBJS = \
    $(OBJDIR)\haltonrsg.obj$(_D) \
    $(OBJDIR)\knuthuniformrng.obj$(_D) \
    $(OBJDIR)\lecuyeruniformrng.obj$(_D) \
    $(OBJDIR)\mt19937uniformrng.obj$(_D) \
    $(OBJDIR)\primitivepolynomials.obj$(_D) \
    $(OBJDIR)\sobolrsg.obj$(_D)

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

TLIB_OPTS    = /P256
!ifdef DEBUG
TLIB_OPTS    = /P256
!endif

# Generic rules
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    $(CC) $(CC_OPTS) $<
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<
{$(SRCDIR)}.c{$(OBJDIR)}.obj:
    $(CC) $(CC_OPTS) -o$@ $<
{$(SRCDIR)}.c{$(OBJDIR)}.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
$(OBJDIR)\RandomNumbers$(_D).lib:: $(OBJDIR) $(OBJS)
    if exist $(OBJDIR)\RandomNumbers$(_D).lib     del $(OBJDIR)\RandomNumbers$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OBJDIR)\RandomNumbers$(_D).lib /a $(OBJS)

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

# Clean up
clean::
    if exist $(OBJDIR)\*.obj         del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.obj$(_D)    del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib         del /q $(OBJDIR)\*.lib
