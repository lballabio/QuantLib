
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/TermStructures

include_HEADERS = \
	piecewiseflatforward.hpp \
	ratehelpers.hpp \
    flatforward.hpp

EXTRA_DIST=makefile.mak

libTermStructures_la_SOURCES = \
    piecewiseflatforward.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libTermStructures.la
