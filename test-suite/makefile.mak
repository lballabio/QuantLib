
# makefile for test-suite.cpp under Borland C++

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
    _bc5D = d
!endif

# Directories
BOOST_INCLUDE_DIR = "$(BOOST_INCLUDE_DIR)"
BOOST_LIB_DIR     = "$(BOOST_LIB_DIR)"
QL_INCLUDE_DIR    = "$(QL_DIR)"
QL_LIB_DIR        = "$(QL_DIR)\lib\Win32\Borland"

# Object files
QL_TESTS = \
    americanoption.obj$(_D) \
    asianoptions.obj$(_D) \
    barrieroption.obj$(_D) \
    basketoption.obj$(_D) \
    calendars.obj$(_D) \
    capfloor.obj$(_D) \
    compoundforward.obj$(_D) \
    covariance.obj$(_D) \
    dates.obj$(_D) \
    daycounters.obj$(_D) \
    digitaloption.obj$(_D) \
    distributions.obj$(_D) \
    europeanoption.obj$(_D) \
    factorial.obj$(_D) \
    instruments.obj$(_D) \
    integrals.obj$(_D) \
    interpolations.obj$(_D) \
    jumpdiffusion.obj$(_D) \
    lowdiscrepancysequences.obj$(_D) \
    matrices.obj$(_D) \
    mersennetwister.obj$(_D) \
    old_pricers.obj$(_D) \
    operators.obj$(_D) \
    piecewiseflatforward.obj$(_D) \
    quantlibtestsuite.obj$(_D) \
    quotes.obj$(_D) \
    riskstats.obj$(_D) \
    solvers.obj$(_D) \
    stats.obj$(_D) \
    swap.obj$(_D) \
    swaption.obj$(_D) \
    termstructures.obj$(_D) \
    utilities.obj$(_D)

# Tools to be used
CC        = bcc32

# Options
CC_OPTS = -vi- \
    -I$(BOOST_INCLUDE_DIR) \
    -I$(QL_INCLUDE_DIR)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

# Generic rules
.cpp.obj:
    $(CC) -c -q $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) -c -q $(CC_OPTS) -o$@ $<


# Primary target:
test-suite$(_D).exe: $(QL_TESTS)
    $(CC) $(CC_OPTS) -L$(QL_LIB_DIR) -L$(BOOST_LIB_DIR) \
    -etest-suite$(_D).exe $(QL_TESTS) \
    QuantLib$(_D).lib libboost_unit_test_framework-bcb-s$(_bc5D)-1_31.lib

# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.tds   del /q *.tds
    if exist *.exe   del /q *.exe
