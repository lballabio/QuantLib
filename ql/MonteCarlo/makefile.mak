
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
    arithmeticapopathpricer.obj$(_D) \
    arithmeticasopathpricer.obj$(_D) \
    barrierpathpricer.obj$(_D) \
    basketpathpricer.obj$(_D) \
    biasedbarrierpathpricer.obj$(_D) \
    cliquetoptionpathpricer.obj$(_D) \
    digitalpathpricer.obj$(_D) \
    europeanpathpricer.obj$(_D) \
    europeanmultipathpricer.obj$(_D) \
    everestpathpricer.obj$(_D) \
    geometricapopathpricer.obj$(_D) \
    geometricasopathpricer.obj$(_D) \
    himalayapathpricer.obj$(_D) \
	maxbasketpathpricer.obj$(_D) \
    pagodapathpricer.obj$(_D) \
	performanceoptionpathpricer.obj$(_D)

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
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
MonteCarlo$(_D).lib:: $(OBJS)
    if exist MonteCarlo$(_D).lib     del MonteCarlo$(_D).lib
    $(TLIB) $(TLIB_OPTS) MonteCarlo$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib         del /q *.lib
