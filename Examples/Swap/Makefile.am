
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = SwapValuation
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = ../../ql/libQuantLib.la

TESTS = SwapValuation@EXEEXT@

EXTRA_DIST = \
    Swap.dsp \
    Swap.mak \
    makefile.mak \
    README.txt
