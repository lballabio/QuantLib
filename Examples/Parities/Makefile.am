CXXFLAGS = -g -O2 -Wall
INCLUDES = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = Parities
Parities_SOURCES = Parities.cpp
Parities_LDADD = ../../ql/libQuantLib.la

TESTS = Parities

EXTRA_DIST = \
    Parities.dep \
    Parities.dsp \
    Parities.mak \
    makefile.mak \
    ReadMe.txt
