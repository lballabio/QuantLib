
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = AmericanOption
else
EXTRA_PROGRAMS = AmericanOption
CLEANFILES = AmericanOption
endif
AmericanOption_SOURCES = AmericanOption.cpp
AmericanOption_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    AmericanOption.dsp \
    AmericanOption.mak \
    AmericanOption.vcproj \
    makefile.mak \
    ReadMe.txt

.PHONY: examples
examples: AmericanOption

