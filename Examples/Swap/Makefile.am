
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = SwapValuation
else
EXTRA_PROGRAMS = SwapValuation
CLEANFILES = SwapValuation
endif
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Swap.dsp \
    Swap.mak \
    Swap.vcproj \
    makefile.mak \
    README.txt

.PHONY: examples
examples: SwapValuation

