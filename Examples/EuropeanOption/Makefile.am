
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = EuropeanOption
else
EXTRA_PROGRAMS = EuropeanOption
CLEANFILES = EuropeanOption
endif
EuropeanOption_SOURCES = EuropeanOption.cpp
EuropeanOption_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    EuropeanOption.dsp \
    EuropeanOption.mak \
    makefile.mak \
    ReadMe.txt

examples: EuropeanOption

