
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = DiscreteHedging
TESTS = DiscreteHedging
else
EXTRA_PROGRAMS = DiscreteHedging
CLEANFILES = DiscreteHedging
endif
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    DiscreteHedging.dev \
    DiscreteHedging_vc7.vcproj \
    DiscreteHedging_vc8.vcproj \
    ReadMe.txt

.PHONY: examples check-examples

examples: DiscreteHedging$(EXEEXT)

check-examples: examples
	./DiscreteHedging$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

