# makefile for test-suite.cpp under Borland C++
#
# $Id$

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
CPPUNIT_INCLUDE_DIR = "$(CPPUNIT_DIR)\include"
CPPUNIT_LIB_DIR     = "$(CPPUNIT_DIR)\lib"
QL_INCLUDE_DIR      = "$(QL_DIR)"
QL_LIB_DIR          = "$(QL_DIR)\lib\Win32\Borland"
BCC_INCLUDE         = $(MAKEDIR)\..\include
BCC_LIBS            = $(MAKEDIR)\..\lib

# Object files
QL_TESTS = \
    calendars.obj$(_D) \
    capfloor.obj$(_D) \
    covariance.obj$(_D) \
    dates.obj$(_D) \
    daycounters.obj$(_D) \
    distributions.obj$(_D) \
    europeanoption.obj$(_D) \
    instruments.obj$(_D) \
    integrals.obj$(_D) \
    lowdiscrepancysequences.obj$(_D) \
    marketelements.obj$(_D) \
    matrices.obj$(_D) \
    mersennetwister.obj$(_D) \
    old_pricers.obj$(_D) \
    operators.obj$(_D) \
    piecewiseflatforward.obj$(_D) \
    qltestlistener.obj$(_D) \
    quantlibtestsuite.obj$(_D) \
    riskstats.obj$(_D) \
    solvers.obj$(_D) \
    stats.obj$(_D) \
    swap.obj$(_D) \
    swaption.obj$(_D) \
    termstructures.obj$(_D)

# Tools to be used
CC        = bcc32

# Options
CC_OPTS = -vi- \
    -I$(QL_INCLUDE_DIR) \
    -I$(CPPUNIT_INCLUDE_DIR) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DSAFE_CHECKS
!endif

# Generic rules
.cpp.obj:
    $(CC) -c -q $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) -c -q $(CC_OPTS) -o$@ $<

# Primary target:
test-suite$(_D).exe: $(QL_TESTS)
    $(CC) $(CC_OPTS) -L$(QL_LIB_DIR) -L$(CPPUNIT_LIB_DIR) -L$(BCC_LIBS) \
    -etest-suite$(_D).exe $(QL_TESTS) \
    QuantLib$(_D).lib cppunit_bcpp$(_D).lib

# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.tds   del /q *.tds
    if exist *.exe   del /q *.exe
