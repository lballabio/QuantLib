
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = DiscreteHedging
else
EXTRA_PROGRAMS = DiscreteHedging
CLEANFILES = DiscreteHedging
endif
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    DiscreteHedging.dsp \
    DiscreteHedging.mak \
    DiscreteHedging.vcproj \
    makefile.mak \
    ReadMe.txt

.PHONY: examples
examples: DiscreteHedging

