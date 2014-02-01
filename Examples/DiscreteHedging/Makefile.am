
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = DiscreteHedging
TESTS = DiscreteHedging$(EXEEXT)
else
noinst_PROGRAMS = DiscreteHedging
endif
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    DiscreteHedging.dev \
    DiscreteHedging_vc8.vcproj \
    DiscreteHedging_vc9.vcproj \
    DiscreteHedging_vc10.vcxproj \
    DiscreteHedging_vc10.vcxproj.filters \
    DiscreteHedging_vc11.vcxproj \
    DiscreteHedging_vc11.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: DiscreteHedging$(EXEEXT)

check-examples: examples
	./DiscreteHedging$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

