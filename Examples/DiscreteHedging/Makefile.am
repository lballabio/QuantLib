
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = DiscreteHedging
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = ../../ql/libQuantLib.la

TESTS = DiscreteHedging@EXEEXT@

EXTRA_DIST = \
    DiscreteHedging.dsp \
    DiscreteHedging.mak \
    makefile.mak \
    ReadMe.txt
