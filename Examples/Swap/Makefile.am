CXXFLAGS = -g -O2 -Wall @QUANTLIB_CXXFLAGS@
INCLUDES = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = SwapValuation
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = @QUANTLIB_LIBS@

TESTS = SwapValuation@EXEEXT@

EXTRA_DIST = \
    Swap.dsp \
    Swap.mak \
    makefile.mak \
    README.txt
