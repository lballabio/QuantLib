
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
OBJS = avgpriceasianpathpricer.obj$(_D) \
       avgstrikeasianpathpricer.obj$(_D) \
       basketpathpricer.obj$(_D) \
       controlvariatedpathpricer.obj$(_D) \
       europeanpathpricer.obj$(_D) \
       everestpathpricer.obj$(_D) \
       geometricasianpathpricer.obj$(_D) \
       getcovariance.obj$(_D) \
       himalayapathpricer.obj$(_D) \
       knuthrandomgenerator.obj$(_D) \
       lecuyerrandomgenerator.obj$(_D) \
       pagodapathpricer.obj$(_D) \
       singleassetpathpricer.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -q -c -tWM -n$(OUTPUT_DIR) \
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
MonteCarlo$(_D).lib:: $(OBJS)
    if exist MonteCarlo$(_D).lib     del MonteCarlo$(_D).lib
    $(TLIB) $(TLIB_OPTS) MonteCarlo$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

