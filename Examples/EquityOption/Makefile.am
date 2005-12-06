
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = EquityOption
TESTS = EquityOption
else
EXTRA_PROGRAMS = EquityOption
CLEANFILES = EquityOption
endif
EquityOption_SOURCES = EquityOption.cpp
EquityOption_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    EquityOption.dev \
    EquityOption.dsp \
    EquityOption.vcproj \
    EquityOption_vc8.vcproj \
    makefile.mak \
    ReadMe.txt

.PHONY: examples check-examples

examples: EquityOption

check-examples: examples
	./EquityOption$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

