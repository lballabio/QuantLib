CXXFLAGS = -g -O3 -pedantic -Wall @QUANTLIB_CXXFLAGS@
INCLUDES = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = EuropeanOption
EuropeanOption_SOURCES = EuropeanOption.cpp
EuropeanOption_LDADD = @QUANTLIB_LIBS@

TESTS = EuropeanOption@EXEEXT@

EXTRA_DIST = \
    EuropeanOption.dsp \
    EuropeanOption.mak \
    makefile.mak \
    ReadMe.txt
