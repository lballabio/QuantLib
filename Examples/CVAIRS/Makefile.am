
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = CVAIRS
TESTS = CVAIRS$(EXEEXT)
else
noinst_PROGRAMS = CVAIRS
endif
CVAIRS_SOURCES = CVAIRS.cpp
CVAIRS_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CVAIRS.dev \
    CVAIRS_vc8.vcproj \
    CVAIRS_vc9.vcproj \
    CVAIRS.vcxproj \
    CVAIRS.vcxproj.filters

.PHONY: examples check-examples

examples: CVAIRS$(EXEEXT)

check-examples: examples
	./CVAIRS$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

