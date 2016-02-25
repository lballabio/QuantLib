
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = FittedBondCurve
TESTS = FittedBondCurve$(EXEEXT)
else
noinst_PROGRAMS = FittedBondCurve
endif
FittedBondCurve_SOURCES = FittedBondCurve.cpp
FittedBondCurve_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    FittedBondCurve.dev \
    FittedBondCurve_vc8.vcproj \
    FittedBondCurve_vc9.vcproj \
    FittedBondCurve.vcxproj \
    FittedBondCurve.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: FittedBondCurve$(EXEEXT)

check-examples: examples
	./FittedBondCurve$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

