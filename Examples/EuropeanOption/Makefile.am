
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = EuropeanOption
EuropeanOption_SOURCES = EuropeanOption.cpp
EuropeanOption_LDADD = ../../ql/libQuantLib.la

TESTS = EuropeanOption

EXTRA_DIST = \
    EuropeanOption.dsp \
    EuropeanOption.mak \
    makefile.mak \
    ReadMe.txt
