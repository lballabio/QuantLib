
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = ConvertibleBonds
TESTS = ConvertibleBonds
else
EXTRA_PROGRAMS = ConvertibleBonds
CLEANFILES = ConvertibleBonds
endif
ConvertibleBonds_SOURCES = ConvertibleBonds.cpp
ConvertibleBonds_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    ConvertibleBonds.dev \
    ConvertibleBonds_vc7.vcproj \
    ConvertibleBonds_vc8.vcproj \
    ReadMe.txt

.PHONY: examples check-examples

examples: ConvertibleBonds$(EXEEXT)

check-examples: examples
	./ConvertibleBonds$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

