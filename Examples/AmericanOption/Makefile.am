
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

EXTRA_PROGRAMS = AmericanOption
AmericanOption_SOURCES = AmericanOption.cpp
AmericanOption_LDADD = ../../ql/libQuantLib.la

CLEANFILES = AmericanOption

EXTRA_DIST = \
    AmericanOption.dsp \
    AmericanOption.mak \
    makefile.mak \
    ReadMe.txt

examples: AmericanOption

