
# $Id$


includedir = $(prefix)/include/ql/TermStructures
include_HEADERS = \
    flatforward.hpp \
	piecewiseflatforward.hpp \
	ratehelpers.hpp \
	piecewiseconstantforwards.hpp

EXTRA_DIST=makefile.mak

libTermStructures_la_SOURCES = \
    piecewiseconstantforwards.cpp \
    piecewiseflatforward.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libTermStructures.la
