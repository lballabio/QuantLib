
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Math

include_HEADERS = \
	bilinearinterpolation.hpp \
	bicubicsplineinterpolation.hpp \
    chisquaredistribution.hpp \
	cubicspline.hpp \
	discrepancystatistic.hpp \
	errorfunction.hpp \
	interpolation.hpp \
	interpolation2D.hpp \
    gammadistribution.hpp \
	hstatistic.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	multivariateaccumulator.hpp \
	normaldistribution.hpp \
	primenumbers.hpp \
	riskmeasures.hpp \
	segmentintegral.hpp \
	sequencestatistic.hpp \
	statistic.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
    chisquaredistribution.cpp \
	errorfunction.cpp \
    gammadistribution.cpp \
	hstatistic.cpp \
    matrix.cpp \
    multivariateaccumulator.cpp \
    normaldistribution.cpp \
	primenumbers.cpp \
    statistic.cpp \
    symmetricschurdecomposition.cpp

noinst_LTLIBRARIES = libMath.la
