CXXFLAGS = -g -O2 -Wall @QUANTLIB_CXXFLAGS@
INCLUDES = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = Parities
Parities_SOURCES = Parities.cpp
Parities_LDADD = @QUANTLIB_LIBS@

TESTS = Parities

EXTRA_DIST = \
    Parities.dsp \
    Parities.mak \
    makefile.mak \
    ReadMe.txt
