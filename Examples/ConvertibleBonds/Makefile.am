
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = ConvertibleBonds
TESTS = ConvertibleBonds$(EXEEXT)
else
noinst_PROGRAMS = ConvertibleBonds
endif
ConvertibleBonds_SOURCES = ConvertibleBonds.cpp
ConvertibleBonds_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    ConvertibleBonds.dev \
    ConvertibleBonds_vc8.vcproj \
    ConvertibleBonds_vc9.vcproj \
    ConvertibleBonds_vc10.vcxproj \
    ConvertibleBonds_vc10.vcxproj.filters \
    ConvertibleBonds_vc11.vcxproj \
    ConvertibleBonds_vc11.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: ConvertibleBonds$(EXEEXT)

check-examples: examples
	./ConvertibleBonds$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

