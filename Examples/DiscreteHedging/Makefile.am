
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

EXTRA_PROGRAMS = DiscreteHedging
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = ../../ql/libQuantLib.la

CLEANFILES = DiscreteHedging

EXTRA_DIST = \
    DiscreteHedging.dsp \
    DiscreteHedging.mak \
    makefile.mak \
    ReadMe.txt

examples: DiscreteHedging

