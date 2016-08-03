
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = LatentModel
TESTS = LatentModel$(EXEEXT)
else
noinst_PROGRAMS = LatentModel
endif
LatentModel_SOURCES = LatentModel.cpp
LatentModel_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    LatentModel.dev \
    LatentModel_vc8.vcproj \
    LatentModel_vc9.vcproj \
    LatentModel.vcxproj \
    LatentModel.vcxproj.filters

.PHONY: examples check-examples

examples: LatentModel$(EXEEXT)

check-examples: examples
	./LatentModel$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

