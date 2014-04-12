
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = SwapValuation
TESTS = SwapValuation$(EXEEXT)
else
noinst_PROGRAMS = SwapValuation
endif
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Swap.dev \
    Swap_vc8.vcproj \
    Swap_vc9.vcproj \
    Swap_vc10.vcxproj \
    Swap_vc10.vcxproj.filters \
    Swap_vc11.vcxproj \
    Swap_vc11.vcxproj.filters \
    README.txt

.PHONY: examples check-examples

examples: SwapValuation$(EXEEXT)

check-examples: examples
	./SwapValuation$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

