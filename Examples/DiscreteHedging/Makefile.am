
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

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

