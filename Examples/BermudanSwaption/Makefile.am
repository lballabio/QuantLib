
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = BermudanSwaption
TESTS = BermudanSwaption
else
EXTRA_PROGRAMS = BermudanSwaption
CLEANFILES = BermudanSwaption
endif
BermudanSwaption_SOURCES = BermudanSwaption.cpp
BermudanSwaption_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    BermudanSwaption.dev \
    BermudanSwaption.dsp \
    BermudanSwaption.vcproj \
    BermudanSwaption_vc8.vcproj \
    makefile.mak \
    ReadMe.txt

.PHONY: examples check-examples

examples: BermudanSwaption

check-examples: examples
	./BermudanSwaption$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

