
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = SwapValuation
TESTS = SwapValuation$(EXEEXT)
else
noinst_PROGRAMS = SwapValuation
endif
SwapValuation_SOURCES = swapvaluation.cpp
SwapValuation_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    Swap.dev \
    Swap_vc8.vcproj \
    Swap_vc9.vcproj \
    Swap.vcxproj \
    Swap.vcxproj.filters \
    README.txt

.PHONY: examples check-examples

examples: SwapValuation$(EXEEXT)

check-examples: examples
	./SwapValuation$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

