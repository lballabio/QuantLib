
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = FittedBondCurve
TESTS = FittedBondCurve$(EXEEXT)
else
EXTRA_PROGRAMS = FittedBondCurve
CLEANFILES = FittedBondCurve$(EXEEXT)
endif
FittedBondCurve_SOURCES = FittedBondCurve.cpp
FittedBondCurve_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    FittedBondCurve.dev \
    FittedBondCurve_vc7.vcproj \
    FittedBondCurve_vc8.vcproj \
    ReadMe.txt

.PHONY: examples check-examples

examples: FittedBondCurve$(EXEEXT)

check-examples: examples
	./FittedBondCurve$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

