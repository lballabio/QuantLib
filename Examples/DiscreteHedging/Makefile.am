
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = DiscreteHedging
TESTS = DiscreteHedging$(EXEEXT)
else
noinst_PROGRAMS = DiscreteHedging
endif
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    DiscreteHedging.dev \
    DiscreteHedging_vc8.vcproj \
    DiscreteHedging_vc9.vcproj \
    DiscreteHedging.vcxproj \
    DiscreteHedging.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: DiscreteHedging$(EXEEXT)

check-examples: examples
	./DiscreteHedging$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

