
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = FRA
TESTS = FRA
else
EXTRA_PROGRAMS = FRA
CLEANFILES = FRA
endif
FRA_SOURCES = FRA.cpp
FRA_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    FRA.dev \
    FRA_vc7.vcproj \
    FRA_vc8.vcproj \
    ReadMe.txt

.PHONY: examples check-examples

examples: FRA$(EXEEXT)

check-examples: examples
	./FRA$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

