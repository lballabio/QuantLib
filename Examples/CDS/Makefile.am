
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = CDS
TESTS = CDS$(EXEEXT)
else
noinst_PROGRAMS = CDS
endif
CDS_SOURCES = CDS.cpp
CDS_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
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

