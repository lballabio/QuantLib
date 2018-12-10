
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = BermudanSwaption
TESTS = BermudanSwaption$(EXEEXT)
else
noinst_PROGRAMS = BermudanSwaption
endif
BermudanSwaption_SOURCES = BermudanSwaption.cpp
BermudanSwaption_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    BermudanSwaption.vcxproj \
    BermudanSwaption.vcxproj.filters \
    CMakeLists.txt \
    ReadMe.txt

.PHONY: examples check-examples

examples: BermudanSwaption$(EXEEXT)

check-examples: examples
	./BermudanSwaption$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

