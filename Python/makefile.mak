
# $Source$

#
# makefile for QuantLib Python module under Borland C++
#

.autodepend
.silent

# Directories
SWIG_DIR       = ..\Swig
INCLUDE_DIR    = ..\Include
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib

!if "$(PYTHON_HOME)" == ""
!message Either set the PYTHON_HOME environment variable to the absolute 
!message path of your Python installation or pass it to $(MAKE) as in
!message
!message $(MAKE) -DPYTHON_HOME="your\Python\path".
!message
!error terminated
!endif
PYTHON_INCLUDE = "$(PYTHON_HOME)"\include
PYTHON_LIBS    = "$(PYTHON_HOME)"\libs

# Object files
!ifdef DEBUG
    WRAPPER_OBJ = quantlib_wrap_d.obj
!else
    WRAPPER_OBJ = quantlib_wrap.obj
!endif
WIN_OBJS        = c0d32.obj

# Libraries
!ifdef DEBUG
    QUANTLIB_LIB = ..\Win\Debug\QuantLib.lib
    QUANTLIB_DLL = QuantLibc_d.dll
!else
    QUANTLIB_LIB = ..\Win\Release\QuantLib.lib
    QUANTLIB_DLL = QuantLibc.dll
!endif
WIN_LIBS         = import32.lib cw32mt.lib
PYTHON_BCC_LIB   = bccpython.lib

# Tools to be used
CC        = bcc32
LINK      = ilink32
COFF2OMF  = coff2omf
SWIG      = swig

# Options
CC_OPTS        = -q -c -tWM \
    -vi- -w-8057 -w-8004 -w-8060 \
    -w-8026 -w-8027 -w-8012 \
    -D__WIN32__ -DMSC_CORE_BC_EXT \
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
    -I$(PYTHON_INCLUDE) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
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
Python: $(QUANTLIB_DLL)

$(QUANTLIB_DLL):: $(WRAPPER_OBJ) $(QUANTLIB_LIB) $(PYTHON_BCC_LIB)
    echo Linking Python module...
    $(LINK) $(LINK_OPTS) \
        $(WRAPPER_OBJ) \
        $(WIN_OBJS), \
        $(QUANTLIB_DLL),, \
        $(QUANTLIB_LIB) $(PYTHON_BCC_LIB) $(WIN_LIBS), \
        QuantLibc.def
    if exist QuantLibc.ilc      del QuantLibc.ilc
    if exist QuantLibc_d.ilc    del QuantLibc_d.ilc
    if exist QuantLibc.ild      del QuantLibc.ild
    if exist QuantLibc_d.ild    del QuantLibc_d.ild
    if exist QuantLibc.ilf      del QuantLibc.ilf
    if exist QuantLibc_d.ilf    del QuantLibc_d.ilf
    if exist QuantLibc.ils      del QuantLibc.ils
    if exist QuantLibc_d.ils    del QuantLibc_d.ils
    echo Build completed

# Python lib in OMF format
$(PYTHON_BCC_LIB):
    if exist $(PYTHON_LIBS)\python15.lib $(COFF2OMF) -q $(PYTHON_LIBS)\python15.lib $(PYTHON_BCC_LIB)
    if exist $(PYTHON_LIBS)\python20.lib $(COFF2OMF) -q $(PYTHON_LIBS)\python20.lib $(PYTHON_BCC_LIB)

# Wrapper functions
$(WRAPPER_OBJ):: quantlib_wrap.cpp
    echo Compiling wrappers...
    $(CC) $(CC_OPTS) -o$(WRAPPER_OBJ) quantlib_wrap.cpp

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
    $(SWIG) -python -c++ -shadow -keyword -opt -I$(SWIG_DIR) \
            -o quantlib_wrap.cpp $(SWIG_DIR)\QuantLib.i


# Clean up
clean::
    if exist $(PYTHON_BCC_LIB)      del $(PYTHON_BCC_LIB)
    if exist quantlib_wrap.cpp      del quantlib_wrap.cpp
    if exist QuantLib.py            del QuantLib.py
    if exist QuantLib.pyc           del QuantLib.pyc
    if exist quantlib_wrap.obj      del quantlib_wrap.obj
    if exist quantlib_wrap_d.obj    del quantlib_wrap_d.obj
    if exist QuantLibc.dll          del QuantLibc.dll
    if exist QuantLibc_d.dll        del QuantLibc_d.dll
    if exist QuantLibc.tds          del QuantLibc.tds
    if exist QuantLibc_d.tds        del QuantLibc_d.tds


# Install Python module
install::
    copy QuantLib.py     "$(PYTHON_HOME)"\QuantLib.py
    copy $(QUANTLIB_DLL) "$(PYTHON_HOME)"\$(QUANTLIB_DLL)

# Test Python module
test::
    cd Tests
    python american_option.py -b
    python barrier_option.py -b
    python binary_option.py -b
    python date.py -b
    python distributions.py -b
    python everest_option.py -b
    python european_option.py -b
    python european_with_dividends.py -b
    python finite_difference_european.py -b
    python get_covariance.py -b
    python himalaya_option.py -b
    python implied_volatility.py -b
    python montecarlo_pricers.py -b
    python plain_basket_option.py -b
    python random_generators.py -b
    python risk_statistics.py -b
    python statistics.py -b
    python term_structures.py -b
    cd ..

