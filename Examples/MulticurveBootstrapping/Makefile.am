
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = MulticurveBootstrapping
TESTS = MulticurveBootstrapping$(EXEEXT)
else
noinst_PROGRAMS = MulticurveBootstrapping
endif
MulticurveBootstrapping_SOURCES = MulticurveBootstrapping.cpp
MulticurveBootstrapping_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    MulticurveBootstrapping.vcxproj \
    MulticurveBootstrapping.vcxproj.filters \
    README.txt

.PHONY: examples check-examples

examples: MulticurveBootstrapping$(EXEEXT)

check-examples: examples
	./MulticurveBootstrapping$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

