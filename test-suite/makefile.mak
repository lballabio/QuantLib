
# makefile for test-suite.cpp under Borland C++

.autodepend
.silent

!ifdef _DEBUG
!ifndef _RTLDLL
    _D = -sd
!else
    _D = -d
!endif
!else
!ifndef _RTLDLL
    _D = -s
!endif
!endif

!ifdef __MT__
    _mt = -mt
!endif

# Directories
QL_INCLUDE_DIR    = "$(QL_DIR)"
QL_LIB_DIR        = "$(QL_DIR)\lib"

# Object files
QL_TESTS = \
    "americanoption.obj$(_mt)$(_D)" \
    "asianoptions.obj$(_mt)$(_D)" \
    "barrieroption.obj$(_mt)$(_D)" \
    "basketoption.obj$(_mt)$(_D)" \
    "calendars.obj$(_mt)$(_D)" \
    "capfloor.obj$(_mt)$(_D)" \
    "compoundforward.obj$(_mt)$(_D)" \
    "covariance.obj$(_mt)$(_D)" \
    "dates.obj$(_mt)$(_D)" \
    "daycounters.obj$(_mt)$(_D)" \
    "digitaloption.obj$(_mt)$(_D)" \
    "distributions.obj$(_mt)$(_D)" \
    "europeanoption.obj$(_mt)$(_D)" \
    "factorial.obj$(_mt)$(_D)" \
    "instruments.obj$(_mt)$(_D)" \
    "integrals.obj$(_mt)$(_D)" \
    "interpolations.obj$(_mt)$(_D)" \
    "jumpdiffusion.obj$(_mt)$(_D)" \
    "lowdiscrepancysequences.obj$(_mt)$(_D)" \
    "matrices.obj$(_mt)$(_D)" \
    "mersennetwister.obj$(_mt)$(_D)" \
    "old_pricers.obj$(_mt)$(_D)" \
    "operators.obj$(_mt)$(_D)" \
    "piecewiseflatforward.obj$(_mt)$(_D)" \
    "quantlibtestsuite.obj$(_mt)$(_D)" \
    "quotes.obj$(_mt)$(_D)" \
    "riskstats.obj$(_mt)$(_D)" \
    "solvers.obj$(_mt)$(_D)" \
    "stats.obj$(_mt)$(_D)" \
    "swap.obj$(_mt)$(_D)" \
    "swaption.obj$(_mt)$(_D)" \
    "termstructures.obj$(_mt)$(_D)" \
    "utilities.obj$(_mt)$(_D)"

# Tools to be used
CC        = bcc32

# Options
CC_OPTS = -vi- -I$(QL_INCLUDE_DIR)

!ifdef _DEBUG
CC_OPTS = $(CC_OPTS) -v -D_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2 -DNDEBUG
!endif

!ifdef _RTLDLL
    CC_OPTS = $(CC_OPTS) -D_RTLDLL
!endif

!ifdef __MT__
    CC_OPTS = $(CC_OPTS) -tWM
!endif

!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

# Generic rules
.cpp.obj:
    $(CC) -c -q $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) -c -q $(CC_OPTS) -o$@ $<


# Primary target:
test-suite$(_mt)$(_D).exe: $(QL_TESTS)
    $(CC) $(CC_OPTS) -L$(QL_LIB_DIR) \
    -e"test-suite$(_mt)$(_D).exe" $(QL_TESTS) \
    libboost_unit_test_framework-bcb$(_mt)$(_D)-1_31.lib


check: test-suite$(_mt)$(_D).exe
    test-suite$(_mt)$(_D) --log_level=messages --build_info=yes
    cd ..


# Clean up
clean::
    if exist *.obj*  del /q *.obj*
    if exist *.tds   del /q *.tds
    if exist *.exe   del /q *.exe
