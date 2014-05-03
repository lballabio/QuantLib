
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = Gaussian1dModels
TESTS = Gaussian1dModels$(EXEEXT)
else
noinst_PROGRAMS = Gaussian1dModels
endif
Gaussian1dModels_SOURCES = Gaussian1dModels.cpp
Gaussian1dModels_LDADD = ../../ql/libQuantLib.la

#EXTRA_DIST =

.PHONY: examples check-examples

examples: Gaussian1dModels$(EXEEXT)

check-examples: examples
	./Gaussian1dModels$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

