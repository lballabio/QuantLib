
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
    $(OBJDIR)\barrieroption.obj$(_D) \
    $(OBJDIR)\blackcapfloor.obj$(_D) \
    $(OBJDIR)\blackswaption.obj$(_D) \
    $(OBJDIR)\fdbermudanoption.obj$(_D) \
    $(OBJDIR)\binaryoption.obj$(_D) \
    $(OBJDIR)\capfloorpricer.obj$(_D) \
    $(OBJDIR)\cliquetoption.obj$(_D) \
    $(OBJDIR)\discretegeometricapo.obj$(_D) \
    $(OBJDIR)\discretegeometricaso.obj$(_D) \
    $(OBJDIR)\fddividendshoutoption.obj$(_D) \
    $(OBJDIR)\europeanoption.obj$(_D) \
    $(OBJDIR)\fdbsmoption.obj$(_D) \
    $(OBJDIR)\fddividendamericanoption.obj$(_D) \
    $(OBJDIR)\fddividendeuropeanoption.obj$(_D) \
    $(OBJDIR)\fddividendoption.obj$(_D) \
    $(OBJDIR)\fdeuropean.obj$(_D) \
    $(OBJDIR)\fdmultiperiodoption.obj$(_D) \
    $(OBJDIR)\fdstepconditionoption.obj$(_D) \
    $(OBJDIR)\jamshidianswaption.obj$(_D) \
    $(OBJDIR)\mcbasket.obj$(_D) \
    $(OBJDIR)\mccliquetoption.obj$(_D) \
    $(OBJDIR)\mcdiscretearithmeticapo.obj$(_D) \
    $(OBJDIR)\mcdiscretearithmeticaso.obj$(_D) \
    $(OBJDIR)\mceuropean.obj$(_D) \
    $(OBJDIR)\mceverest.obj$(_D) \
    $(OBJDIR)\mchimalaya.obj$(_D) \
    $(OBJDIR)\mcmaxbasket.obj$(_D) \
    $(OBJDIR)\mcpagoda.obj$(_D) \
    $(OBJDIR)\mcperformanceoption.obj$(_D) \
    $(OBJDIR)\performanceoption.obj$(_D) \
    $(OBJDIR)\singleassetoption.obj$(_D) \
    $(OBJDIR)\swaptionpricer.obj$(_D) \
    $(OBJDIR)\treecapfloor.obj$(_D) \
    $(OBJDIR)\treeswaption.obj$(_D)

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
TLIB_OPTS    = /P256
!endif

# Generic rules
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    $(CC) $(CC_OPTS) $<
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
$(OBJDIR)\Pricers$(_D).lib:: $(OBJDIR) $(OBJS)
    if exist $(OBJDIR)\Pricers$(_D).lib     del $(OBJDIR)\Pricers$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OBJDIR)\Pricers$(_D).lib /a $(OBJS)

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

# Clean up
clean::
    if exist $(OBJDIR)\*.obj         del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.obj$(_D)    del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib         del /q $(OBJDIR)\*.lib
