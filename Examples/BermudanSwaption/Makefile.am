CXXFLAGS = -g -O3 -pedantic -Wall @QUANTLIB_CXXFLAGS@
INCLUDES = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = BermudanSwaption
BermudanSwaption_SOURCES = BermudanSwaption.cpp
BermudanSwaption_LDADD = @QUANTLIB_LIBS@

TESTS = BermudanSwaption@EXEEXT@

EXTRA_DIST = \
    BermudanSwaption.dsp \
    BermudanSwaption.mak \
    makefile.mak \
    ReadMe.txt
