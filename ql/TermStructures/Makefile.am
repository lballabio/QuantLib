
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/TermStructures

include_HEADERS = \
	piecewiseconstantforwards.hpp \
	piecewiseflatforward.hpp \
	ratehelpers.hpp \
    flatforward.hpp

EXTRA_DIST=makefile.mak

libTermStructures_la_SOURCES = \
    piecewiseconstantforwards.cpp \
    piecewiseflatforward.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libTermStructures.la
