
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = ConvertibleBond
TESTS = ConvertibleBond
else
EXTRA_PROGRAMS = ConvertibleBond
CLEANFILES = ConvertibleBond
endif
ConvertibleBond_SOURCES = ConvertibleBonds.cpp utilities.cpp
ConvertibleBond_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST =

.PHONY: examples check-examples

examples: ConvertibleBond

check-examples: examples
	./ConvertibleBond$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

