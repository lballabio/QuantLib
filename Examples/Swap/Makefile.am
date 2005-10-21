
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = SwapValuation
TESTS = SwapValuation
else
EXTRA_PROGRAMS = SwapValuation
CLEANFILES = SwapValuation
endif
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Swap.dev \
    Swap.dsp \
    Swap.vcproj \
    Swap_vc8.vcproj \
    makefile.mak \
    README.txt

.PHONY: examples check-examples

examples: SwapValuation

check-examples: examples
	./SwapValuation$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

