
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
       averagepriceasian.obj$(_D) \
       averagestrikeasian.obj$(_D) \
       barrieroption.obj$(_D) \
       basket.obj$(_D) \
       bermudanoption.obj$(_D) \
       binaryoption.obj$(_D) \
       bsmnumericaloption.obj$(_D) \
       cliquetoption.obj$(_D) \
       dividendamericanoption.obj$(_D) \
       dividendeuropeanoption.obj$(_D) \
       dividendoption.obj$(_D) \
       dividendshoutoption.obj$(_D) \
       europeanengine.obj$(_D) \
       europeanoption.obj$(_D) \
       everest.obj$(_D) \
       finitedifferenceeuropean.obj$(_D) \
       himalaya.obj$(_D) \
       mceuropean.obj$(_D) \
       multiperiodoption.obj$(_D) \
       pagoda.obj$(_D) \
       singleassetoption.obj$(_D) \
       stepconditionoption.obj$(_D)

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

TLIB_OPTS    = /P32
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

