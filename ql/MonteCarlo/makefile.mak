
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
    $(OBJDIR)\arithmeticapopathpricer.obj$(_D) \
    $(OBJDIR)\arithmeticasopathpricer.obj$(_D) \
    $(OBJDIR)\basketpathpricer.obj$(_D) \
    $(OBJDIR)\cliquetoptionpathpricer.obj$(_D) \
    $(OBJDIR)\europeanpathpricer.obj$(_D) \
    $(OBJDIR)\everestpathpricer.obj$(_D) \
    $(OBJDIR)\geometricapopathpricer.obj$(_D) \
    $(OBJDIR)\geometricasopathpricer.obj$(_D) \
    $(OBJDIR)\himalayapathpricer.obj$(_D) \
	$(OBJDIR)\maxbasketpathpricer.obj$(_D) \
    $(OBJDIR)\pagodapathpricer.obj$(_D) \
	$(OBJDIR)\performanceoptionpathpricer.obj$(_D)

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
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
$(OBJDIR)\MonteCarlo$(_D).lib:: $(OBJDIR) $(OBJS)
    if exist $(OBJDIR)\MonteCarlo$(_D).lib     del $(OBJDIR)\MonteCarlo$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OBJDIR)\MonteCarlo$(_D).lib /a $(OBJS)

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

# Clean up
clean::
    if exist $(OBJDIR)\*.obj         del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.obj$(_D)    del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib         del /q $(OBJDIR)\*.lib
