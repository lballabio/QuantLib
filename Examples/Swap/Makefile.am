
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

EXTRA_PROGRAMS = SwapValuation
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = ../../ql/libQuantLib.la

CLEANFILES = SwapValuation

EXTRA_DIST = \
    Swap.dsp \
    Swap.mak \
    makefile.mak \
    README.txt

examples: SwapValuation

