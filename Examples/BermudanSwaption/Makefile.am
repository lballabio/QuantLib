
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

EXTRA_PROGRAMS = BermudanSwaption
BermudanSwaption_SOURCES = BermudanSwaption.cpp
BermudanSwaption_LDADD = ../../ql/libQuantLib.la

CLEANFILES = BermudanSwaption

EXTRA_DIST = \
    BermudanSwaption.dsp \
    BermudanSwaption.mak \
    makefile.mak \
    ReadMe.txt

examples: BermudanSwaption

