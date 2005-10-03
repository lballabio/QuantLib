
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
    AmericanOption.dev \
    AmericanOption.dsp \
    AmericanOption.vcproj \
    AmericanOption_vc8.vcproj \
    makefile.mak \
    ReadMe.txt

.PHONY: examples
examples: AmericanOption

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

