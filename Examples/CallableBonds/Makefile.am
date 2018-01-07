
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = CallableBonds
TESTS = CallableBonds$(EXEEXT)
else
noinst_PROGRAMS = CallableBonds
endif
CallableBonds_SOURCES = CallableBonds.cpp
CallableBonds_LDADD = ../../ql/libQuantLib.la  ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CallableBonds_vc9.vcproj \
    CallableBonds.vcxproj \
    CallableBonds.vcxproj.filters \
    CMakeLists.txt \
    ReadMe.txt

.PHONY: examples check-examples

examples: CallableBonds$(EXEEXT)

check-examples: examples
	./CallableBonds$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

