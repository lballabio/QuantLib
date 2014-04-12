
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = MarketModels
TESTS = MarketModels$(EXEEXT)
else
noinst_PROGRAMS = MarketModels
endif
MarketModels_SOURCES = MarketModels.cpp
MarketModels_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    MarketModels.dev \
    MarketModels_vc8.vcproj \
    MarketModels_vc9.vcproj \
    MarketModels_vc10.vcxproj \
    MarketModels_vc10.vcxproj.filters \
    MarketModels_vc11.vcxproj \
    MarketModels_vc11.vcxproj.filters

.PHONY: examples check-examples

examples: MarketModels$(EXEEXT)

check-examples: examples
	./MarketModels$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

