
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/TermStructures

include_HEADERS = \
    affinetermstructure.hpp \
    compoundforward.hpp \
    discountcurve.hpp \
    flatforward.hpp \
    forwardspreadedtermstructure.hpp \
    impliedtermstructure.hpp \
    piecewiseflatforward.hpp \
    ratehelpers.hpp \
    zerospreadedtermstructure.hpp

EXTRA_DIST=makefile.mak

libTermStructures_la_SOURCES = \
    affinetermstructure.cpp \
    compoundforward.cpp \
    discountcurve.cpp \
    piecewiseflatforward.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libTermStructures.la
