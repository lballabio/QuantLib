
# $Id$

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib

# Object files
OBJS = \
       barrieroption.obj$(_D) \
       fdbermudanoption.obj$(_D) \
       binaryoption.obj$(_D) \
       cliquetoption.obj$(_D) \
       couponbondoption.obj$(_D) \
       discretegeometricapo.obj$(_D) \
       discretegeometricaso.obj$(_D) \
       fddividendshoutoption.obj$(_D) \
       europeanengine.obj$(_D) \
       europeanoption.obj$(_D) \
       fdbsmoption.obj$(_D) \
       fddividendamericanoption.obj$(_D) \
       fddividendeuropeanoption.obj$(_D) \
       fddividendoption.obj$(_D) \
       fdeuropean.obj$(_D) \
       fdeuropeanswaption.obj$(_D) \
       fdmultiperiodoption.obj$(_D) \
       fdstepconditionoption.obj$(_D) \
       mcbasket.obj$(_D) \
       mcdiscretearithmeticapo.obj$(_D) \
       mcdiscretearithmeticaso.obj$(_D) \
       mceuropean.obj$(_D) \
       mceverest.obj$(_D) \
       mchimalaya.obj$(_D) \
       mcmaxbasket.obj$(_D) \
       mcpagoda.obj$(_D) \
       singleassetoption.obj$(_D)

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

TLIB_OPTS    = /P128
!ifdef DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj_d:
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
Pricers$(_D).lib:: $(OBJS)
    if exist Pricers$(_D).lib     del Pricers$(_D).lib
    $(TLIB) $(TLIB_OPTS) Pricers$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

