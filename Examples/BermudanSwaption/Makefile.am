
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = BermudanSwaption
TESTS = BermudanSwaption$(EXEEXT)
else
noinst_PROGRAMS = BermudanSwaption
endif
BermudanSwaption_SOURCES = BermudanSwaption.cpp
BermudanSwaption_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    BermudanSwaption.dev \
    BermudanSwaption_vc8.vcproj \
    BermudanSwaption_vc9.vcproj \
    BermudanSwaption_vc10.vcxproj \
    BermudanSwaption_vc10.vcxproj.filters \
    BermudanSwaption_vc11.vcxproj \
    BermudanSwaption_vc11.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: BermudanSwaption$(EXEEXT)

check-examples: examples
	./BermudanSwaption$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

