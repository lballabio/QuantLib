CXXFLAGS = @CXXFLAGS@ @QUANTLIB_CXXFLAGS@
INCLUDES = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = DiscreteHedging
DiscreteHedging_SOURCES = DiscreteHedging.cpp
DiscreteHedging_LDADD = @QUANTLIB_LIBS@

TESTS = DiscreteHedging@EXEEXT@

EXTRA_DIST = \
    DiscreteHedging.dsp \
    DiscreteHedging.mak \
    makefile.mak \
    ReadMe.txt
