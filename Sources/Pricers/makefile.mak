
# $Source$

#
# makefile for QuantLib pricer library under Borland C++
#

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..\Include
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib

# Object files
OBJS = averagepriceasian.obj$(_D) \
       averagestrikeasian.obj$(_D) \
       barrieroption.obj$(_D) \
       bermudanoption.obj$(_D) \
       binaryoption.obj$(_D) \
       bsmeuropeanoption.obj$(_D) \
       bsmnumericaloption.obj$(_D) \
       bsmoption.obj$(_D) \
       dividendamericanoption.obj$(_D) \
       dividendeuropeanoption.obj$(_D) \
       dividendoption.obj$(_D) \
       dividendshoutoption.obj$(_D) \
       everestoption.obj$(_D) \
       finitedifferenceeuropean.obj$(_D) \
       himalaya.obj$(_D) \
       mceuropeanpricer.obj$(_D) \
       multiperiodoption.obj$(_D) \
       pagodaoption.obj$(_D) \
       plainbasketoption.obj$(_D) \
       stepconditionoption.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -q -c -tWM -n$(OUTPUT_DIR) \
    -w-8026 -w-8027 -w-8012 \
    -I$(INCLUDE_DIR) \
    -I$(INCLUDE_DIR)\Calendars \
    -I$(INCLUDE_DIR)\Currencies \
    -I$(INCLUDE_DIR)\DayCounters \
    -I$(INCLUDE_DIR)\FiniteDifferences \
    -I$(INCLUDE_DIR)\Instruments \
    -I$(INCLUDE_DIR)\Math \
    -I$(INCLUDE_DIR)\MonteCarlo \
    -I$(INCLUDE_DIR)\Patterns \
    -I$(INCLUDE_DIR)\Pricers \
    -I$(INCLUDE_DIR)\Solvers1D \
    -I$(INCLUDE_DIR)\TermStructures \
    -I$(INCLUDE_DIR)\Utilities \
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
Pricers$(_D).lib:: Message $(OBJS)
    if exist Pricers$(_D).lib     del Pricers$(_D).lib
    $(TLIB) $(TLIB_OPTS) Pricers$(_D).lib /a $(OBJS)

Message:
    echo Building pricer library...

averagepriceasian.obj$(_D):
averagestrikeasian.obj$(_D):
barrieroption.obj$(_D):
bermudanoption.obj$(_D):
binaryoption.obj$(_D):
bsmeuropeanoption.obj$(_D):
bsmnumericaloption.obj$(_D):
bsmoption.obj$(_D):
dividendamericanoption.obj$(_D):
dividendeuropeanoption.obj$(_D):
dividendoption.obj$(_D):
dividendshoutoption.obj$(_D):
everestoption.obj$(_D):
finitedifferenceeuropean.obj$(_D):
himalaya.obj$(_D):
mceuropeanpricer.obj$(_D):
multiperiodoption.obj$(_D):
pagodaoption.obj$(_D):
plainbasketoption.obj$(_D):
stepconditionoption.obj$(_D):

# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

