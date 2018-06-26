
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = GlobalOptimizer
TESTS = GlobalOptimizer$(EXEEXT)
else
noinst_PROGRAMS = GlobalOptimizer
endif
GlobalOptimizer_SOURCES = GlobalOptimizer.cpp
GlobalOptimizer_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    GlobalOptimizer.vcxproj \
    GlobalOptimizer.vcxproj.filters \
    CMakeLists.txt \
    ReadMe.txt

.PHONY: examples check-examples

examples: GlobalOptimizer$(EXEEXT)

check-examples: examples
	./GlobalOptimizer$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

