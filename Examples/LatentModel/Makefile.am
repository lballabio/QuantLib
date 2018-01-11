
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

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

