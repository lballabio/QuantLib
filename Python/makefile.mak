
# $Id$
# $Source$
#
# makefile for QuantLib Python module under Borland C++
#

.autodepend
.silent

# Debug versions
!ifdef DEBUG
    _D = _d
!endif

# Directories
!if "$(QL_DIR)" == ""
!message Set the QL_DIR environment variable to the absolute
!message path of where you want to install QuantLib before running $(MAKE).
!message
!error terminated
!endif
QL_INCLUDE  = "$(QL_DIR)\Include"

SWIG_DIR    = ..\Swig
BCC_INCLUDE = $(MAKEDIR)\..\include
BCC_LIBS    = $(MAKEDIR)\..\lib

!if "$(PYTHON_HOME)" == ""
!message Set the PYTHON_HOME environment variable to the absolute
!message path of your Python installation before running $(MAKE).
!message
!error terminated
!endif
PYTHON_INCLUDE = "$(PYTHON_HOME)"\include
PYTHON_LIBS    = "$(PYTHON_HOME)"\libs

# Object files
WIN_OBJS        = c0d32.obj

# Libraries
QUANTLIB_LIB     = "$(QL_DIR)\lib\Win32\Borland\QuantLib$(_D).lib"
QUANTLIB_DLL     = QuantLibc$(_D).dll
WIN_LIBS         = import32.lib cw32mt.lib
PYTHON_BCC_LIB   = bccpython$(_D).lib

# Tools to be used
CC        = bcc32
LINK      = ilink32
COFF2OMF  = coff2omf
SWIG      = swig

# Options
CC_OPTS        = -q -c -tWM -vi- \
    -w-8057 -w-8004 -w-8060 \
    -w-8026 -w-8027 -w-8012 \
    -D__WIN32__ -DMSC_CORE_BC_EXT \
    -I$(QL_INCLUDE) \
    -I$(PYTHON_INCLUDE) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DSAFE_CHECKS
!endif

LINK_OPTS    = -q -x -L$(BCC_LIBS) -Tpd
!ifdef DEBUG
LINK_OPTS    = $(LINK_OPTS) -v
!endif


# Generic rules
.cpp.obj:
    rem never mind

# Primary target:
# Python module
python: $(QUANTLIB_DLL)

$(QUANTLIB_DLL):: quantlib_wrap$(_D).obj \
    $(QUANTLIB_LIB) \
    $(PYTHON_BCC_LIB)
    echo Linking Python module...
    $(LINK) $(LINK_OPTS) \
        quantlib_wrap$(_D).obj $(WIN_OBJS), \
        $(QUANTLIB_DLL),, \
        $(QUANTLIB_LIB) $(PYTHON_BCC_LIB) $(WIN_LIBS), \
        QuantLibc.def
    if exist QuantLibc.ilc     del QuantLibc.ilc
    if exist QuantLibc_d.ilc   del QuantLibc_d.ilc
    if exist QuantLibc.ild     del QuantLibc.ild
    if exist QuantLibc_d.ild   del QuantLibc_d.ild
    if exist QuantLibc.ilf     del QuantLibc.ilf
    if exist QuantLibc_d.ilf   del QuantKitc_d.ilf
    if exist QuantLibc.ils     del QuantLibc.ils
    if exist QuantLibc_d.ils   del QuantLibc_d.ils
    if exist QuantLibc.tds del QuantLibc.tds
    echo Build completed

# Python lib in OMF format
$(PYTHON_BCC_LIB):
    if exist $(PYTHON_LIBS)\python15$(_D).lib \
        $(COFF2OMF) -q $(PYTHON_LIBS)\python15$(_D).lib $(PYTHON_BCC_LIB)
    if exist $(PYTHON_LIBS)\python16$(_D).lib \
        $(COFF2OMF) -q $(PYTHON_LIBS)\python16$(_D).lib $(PYTHON_BCC_LIB)
    if exist $(PYTHON_LIBS)\python20$(_D).lib \
        $(COFF2OMF) -q $(PYTHON_LIBS)\python20$(_D).lib $(PYTHON_BCC_LIB)
    if exist $(PYTHON_LIBS)\python21$(_D).lib \
        $(COFF2OMF) -q $(PYTHON_LIBS)\python21$(_D).lib $(PYTHON_BCC_LIB)

# Wrapper functions
quantlib_wrap$(_D).obj:: quantlib_wrap.cpp
    echo Compiling wrappers...
    $(CC) $(CC_OPTS) -oquantlib_wrap$(_D).obj quantlib_wrap.cpp

quantlib_wrap.cpp:: \
    $(SWIG_DIR)\QuantLib.i \
    $(SWIG_DIR)\Barrier.i \
    $(SWIG_DIR)\BoundaryConditions.i \
    $(SWIG_DIR)\Calendars.i \
    $(SWIG_DIR)\Currencies.i \
    $(SWIG_DIR)\Date.i \
    $(SWIG_DIR)\DayCounters.i \
    $(SWIG_DIR)\Distributions.i \
    $(SWIG_DIR)\Financial.i \
    $(SWIG_DIR)\History.i \
    $(SWIG_DIR)\Instruments.i \
    $(SWIG_DIR)\Interpolation.i \
    $(SWIG_DIR)\Matrix.i \
    $(SWIG_DIR)\MontecarloPricers.i \
    $(SWIG_DIR)\MontecarloTools.i \
    $(SWIG_DIR)\Operators.i \
    $(SWIG_DIR)\Options.i \
    $(SWIG_DIR)\Pricers.i \
    $(SWIG_DIR)\QLArray.i \
    $(SWIG_DIR)\RandomGenerators.i \
    $(SWIG_DIR)\RiskStatistics.i \
    $(SWIG_DIR)\Solvers1D.i \
    $(SWIG_DIR)\Statistics.i \
    $(SWIG_DIR)\String.i \
    $(SWIG_DIR)\TermStructures.i \
    $(SWIG_DIR)\Vectors.i
    echo Generating wrappers...
    $(SWIG) -python -c++ -shadow -keyword -opt \
            -I$(SWIG_DIR) \
            -o quantlib_wrap.cpp $(SWIG_DIR)\QuantLib.i

# Clean up
clean::
    if exist bccpython_d.lib        del bccpython_d.lib
    if exist bccpython.lib          del bccpython.lib

    if exist quantlib_wrap.cpp      del quantlib_wrap.cpp
    if exist QuantLib.py            del QuantLib.py
    if exist QuantLib.pyc           del QuantLib.pyc

    if exist quantlib_wrap.obj      del quantlib_wrap.obj
    if exist quantlib_wrap_d.obj    del quantlib_wrap_d.obj

    if exist QuantLibc.dll          del QuantLibc.dll
    if exist QuantLibc.tds          del QuantLibc.tds

    if exist QuantLibc_d.dll        del QuantLibc_d.dll
    if exist QuantLibc_d.tds        del QuantLibc_d.tds
    if exist QuantLibc_d.ilc        del QuantLibc_d.ilc
    if exist QuantLibc_d.ild        del QuantLibc_d.ild
    if exist QuantLibc_d.ilf        del QuantLibc_d.ilf
    if exist QuantLibc_d.ils        del QuantLibc_d.ils

    cd Tests
    if exist *.pyc              del *.pyc

# Install Python module
install::
    del "$(PYTHON_HOME)"\QuantLib*.*
    copy QuantLib.py     "$(PYTHON_HOME)"\QuantLib.py
    copy $(QUANTLIB_DLL) "$(PYTHON_HOME)"\$(QUANTLIB_DLL)

# Test Python module
test::
    cd Tests
    python QuantLibSuite.py
    cd ..

