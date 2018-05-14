
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = Gaussian1dModels
TESTS = Gaussian1dModels$(EXEEXT)
else
noinst_PROGRAMS = Gaussian1dModels
endif
Gaussian1dModels_SOURCES = Gaussian1dModels.cpp
Gaussian1dModels_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    Gaussian1dModels_vc9.vcproj \
    Gaussian1dModels.vcxproj \
    Gaussian1dModels.vcxproj.filters

.PHONY: examples check-examples

examples: Gaussian1dModels$(EXEEXT)

check-examples: examples
	./Gaussian1dModels$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

