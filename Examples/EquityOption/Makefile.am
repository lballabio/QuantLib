
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

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
    EquityOption.dev \
    EquityOption_vc8.vcproj \
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

