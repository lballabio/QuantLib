# makefile for test-suite.cpp under Borland C++

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
    _bc5D = d
!endif

# Directories
CPPUNIT_INCLUDE_DIR = "$(CPPUNIT_DIR)\include"
CPPUNIT_LIB_DIR     = "$(CPPUNIT_DIR)\lib"
QL_INCLUDE_DIR      = "$(QL_DIR)"
QL_LIB_DIR          = "$(QL_DIR)\lib\Win32\Borland"
BCC_INCLUDE         = $(MAKEDIR)\..\include
BCC_LIBS            = $(MAKEDIR)\..\lib
SRCDIR              = "."
OBJDIR              = ".\build\Borland"

# Object files
QL_TESTS = \
    $(OBJDIR)\calendars.obj$(_D) \
    $(OBJDIR)\capfloor.obj$(_D) \
    $(OBJDIR)\compoundforward.obj$(_D) \
    $(OBJDIR)\covariance.obj$(_D) \
    $(OBJDIR)\dates.obj$(_D) \
    $(OBJDIR)\daycounters.obj$(_D) \
    $(OBJDIR)\distributions.obj$(_D) \
    $(OBJDIR)\europeanoption.obj$(_D) \
    $(OBJDIR)\instruments.obj$(_D) \
    $(OBJDIR)\integrals.obj$(_D) \
    $(OBJDIR)\lowdiscrepancysequences.obj$(_D) \
    $(OBJDIR)\marketelements.obj$(_D) \
    $(OBJDIR)\matrices.obj$(_D) \
    $(OBJDIR)\mersennetwister.obj$(_D) \
    $(OBJDIR)\old_pricers.obj$(_D) \
    $(OBJDIR)\operators.obj$(_D) \
    $(OBJDIR)\piecewiseflatforward.obj$(_D) \
    $(OBJDIR)\qltestlistener.obj$(_D) \
    $(OBJDIR)\quantlibtestsuite.obj$(_D) \
    $(OBJDIR)\riskstats.obj$(_D) \
    $(OBJDIR)\solvers.obj$(_D) \
    $(OBJDIR)\stats.obj$(_D) \
    $(OBJDIR)\swap.obj$(_D) \
    $(OBJDIR)\swaption.obj$(_D) \
    $(OBJDIR)\termstructures.obj$(_D)

# Tools to be used
CC        = bcc32

# Options
CC_OPTS = -vi- \
    -I$(QL_INCLUDE_DIR) \
    -I$(CPPUNIT_INCLUDE_DIR) \
    -I$(BCC_INCLUDE) \
    -n$(OBJDIR)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

# Generic rules
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    $(CC) -c -q $(CC_OPTS) $<
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj$(_D):
    $(CC) -c -q $(CC_OPTS) -o$@ $<


# Primary target:
test-suite$(_D).exe: $(OBJDIR) $(QL_TESTS)
    $(CC) $(CC_OPTS) -L$(QL_LIB_DIR) -L$(CPPUNIT_LIB_DIR) -L$(BCC_LIBS) \
    -etest-suite$(_D).exe $(QL_TESTS) \
    QuantLib$(_D).lib cppunit$(_bc5D)_bc5.lib

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

# Clean up
clean::
    if exist $(OBJDIR)\*.obj   del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.tds   del /q $(OBJDIR)\*.tds
    if exist $(OBJDIR)\*.exe   del /q $(OBJDIR)\*.exe
