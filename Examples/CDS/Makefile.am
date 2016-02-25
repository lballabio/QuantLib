
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = CDS
TESTS = CDS$(EXEEXT)
else
noinst_PROGRAMS = CDS
endif
CDS_SOURCES = CDS.cpp
CDS_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CDS.dev \
    CDS_vc8.vcproj \
    CDS_vc9.vcproj \
    CDS.vcxproj \
    CDS.vcxproj.filters \
    CMakeLists.txt \
    ReadMe.txt

.PHONY: examples check-examples

examples: CDS$(EXEEXT)

check-examples: examples
	./CDS$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

