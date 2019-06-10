
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = CVAIRS
TESTS = CVAIRS$(EXEEXT)
else
noinst_PROGRAMS = CVAIRS
endif
CVAIRS_SOURCES = CVAIRS.cpp
CVAIRS_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    CVAIRS.vcxproj \
    CVAIRS.vcxproj.filters

.PHONY: examples check-examples

examples: CVAIRS$(EXEEXT)

check-examples: examples
	./CVAIRS$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

