
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = EquityOption
TESTS = EquityOption$(EXEEXT)
else
noinst_PROGRAMS = EquityOption
endif
EquityOption_SOURCES = EquityOption.cpp
EquityOption_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    EquityOption_vc9.vcproj \
    EquityOption.vcxproj \
    EquityOption.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: EquityOption$(EXEEXT)

check-examples: examples
	./EquityOption$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

