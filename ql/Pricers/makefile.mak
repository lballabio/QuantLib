
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
    cliquetoption.obj$(_D) \
    discretegeometricapo.obj$(_D) \
    discretegeometricaso.obj$(_D) \
    europeanoption.obj$(_D) \
    fdbermudanoption.obj$(_D) \
    fdbsmoption.obj$(_D) \
    fddividendamericanoption.obj$(_D) \
    fddividendeuropeanoption.obj$(_D) \
    fddividendoption.obj$(_D) \
    fddividendshoutoption.obj$(_D) \
    fdeuropean.obj$(_D) \
    fdmultiperiodoption.obj$(_D) \
    fdstepconditionoption.obj$(_D) \
    mcbasket.obj$(_D) \
    mccliquetoption.obj$(_D) \
    mcdiscretearithmeticapo.obj$(_D) \
    mcdiscretearithmeticaso.obj$(_D) \
    mceverest.obj$(_D) \
    mchimalaya.obj$(_D) \
    mcmaxbasket.obj$(_D) \
    mcpagoda.obj$(_D) \
    mcperformanceoption.obj$(_D) \
    performanceoption.obj$(_D) \
    singleassetoption.obj$(_D)

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
TLIB_OPTS    = /P256
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
Pricers$(_D).lib:: $(OBJS)
    if exist Pricers$(_D).lib     del Pricers$(_D).lib
    $(TLIB) $(TLIB_OPTS) Pricers$(_D).lib /a $(OBJS)



# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj$(_D)
    if exist *.lib         del /q *.lib
