
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = BasketLosses
TESTS = BasketLosses$(EXEEXT)
else
noinst_PROGRAMS = BasketLosses
endif
BasketLosses_SOURCES = BasketLosses.cpp
BasketLosses_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    BasketLosses.dev \
    BasketLosses_vc8.vcproj \
    BasketLosses_vc9.vcproj \
    BasketLosses.vcxproj \
    BasketLosses.vcxproj.filters

.PHONY: examples check-examples

examples: BasketLosses$(EXEEXT)

check-examples: examples
	./BasketLosses$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

