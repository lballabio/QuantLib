
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = Bonds
TESTS = Bonds$(EXEEXT)
else
noinst_PROGRAMS = Bonds
endif
Bonds_SOURCES = Bonds.cpp
Bonds_LDADD = ../../ql/libQuantLib.la  ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    Bonds.vcxproj \
    Bonds.vcxproj.filters \
    CMakeLists.txt \
    ReadMe.txt

.PHONY: examples check-examples

examples: Bonds$(EXEEXT)

check-examples: examples
	./Bonds$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

