
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/TermStructures

include_HEADERS = \
    affinetermstructure.hpp \
    compoundforward.hpp \
    discountcurve.hpp \
    drifttermstructure.hpp \
    flatforward.hpp \
    forwardspreadedtermstructure.hpp \
    impliedtermstructure.hpp \
    piecewiseflatforward.hpp \
    quantotermstructure.hpp \
    ratehelpers.hpp \
    zerocurve.hpp \
    zerospreadedtermstructure.hpp

EXTRA_DIST=makefile.mak

libTermStructures_la_SOURCES = \
    affinetermstructure.cpp \
    compoundforward.cpp \
    discountcurve.cpp \
    piecewiseflatforward.cpp \
    ratehelpers.cpp \
    zerocurve.cpp

noinst_LTLIBRARIES = libTermStructures.la
