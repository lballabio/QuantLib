
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

EXTRA_PROGRAMS = EuropeanOption
EuropeanOption_SOURCES = EuropeanOption.cpp
EuropeanOption_LDADD = ../../ql/libQuantLib.la

CLEANFILES = EuropeanOption

EXTRA_DIST = \
    EuropeanOption.dsp \
    EuropeanOption.mak \
    makefile.mak \
    ReadMe.txt

examples: EuropeanOption

