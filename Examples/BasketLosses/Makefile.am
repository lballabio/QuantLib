
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = BasketLosses
TESTS = BasketLosses$(EXEEXT)
else
noinst_PROGRAMS = BasketLosses
endif
BasketLosses_SOURCES = BasketLosses.cpp
BasketLosses_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    BasketLosses.vcxproj \
    BasketLosses.vcxproj.filters \
    CMakeLists.txt

.PHONY: examples check-examples

examples: BasketLosses$(EXEEXT)

check-examples: examples
	./BasketLosses$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

