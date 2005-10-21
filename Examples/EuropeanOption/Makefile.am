
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = EuropeanOption
TESTS = EuropeanOption
else
EXTRA_PROGRAMS = EuropeanOption
CLEANFILES = EuropeanOption
endif
EuropeanOption_SOURCES = EuropeanOption.cpp
EuropeanOption_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    EuropeanOption.dev \
    EuropeanOption.dsp \
    EuropeanOption.vcproj \
    EuropeanOption_vc8.vcproj \
    makefile.mak \
    ReadMe.txt

.PHONY: examples check-examples

examples: EuropeanOption

check-examples: examples
	./EuropeanOption$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

