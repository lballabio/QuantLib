
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = FRA
TESTS = FRA$(EXEEXT)
else
noinst_PROGRAMS = FRA
endif
FRA_SOURCES = FRA.cpp
FRA_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    FRA_vc9.vcproj \
    FRA.vcxproj \
    FRA.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: FRA$(EXEEXT)

check-examples: examples
	./FRA$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

