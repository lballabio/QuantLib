
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Math

include_HEADERS = \
	bilinearinterpolation.hpp \
	bicubicsplineinterpolation.hpp \
    chisquaredistribution.hpp \
	cubicspline.hpp \
	discrepancystatistics.hpp \
	errorfunction.hpp \
	interpolation.hpp \
	interpolation2D.hpp \
    gammadistribution.hpp \
	gaussianstatistics.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	multivariateaccumulator.hpp \
	normaldistribution.hpp \
	primenumbers.hpp \
	riskmeasures.hpp \
	segmentintegral.hpp \
	sequencestatistics.hpp \
	statistics.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
    chisquaredistribution.cpp \
	discrepancystatistics.cpp \
	errorfunction.cpp \
    gammadistribution.cpp \
	gaussianstatistics.cpp \
    matrix.cpp \
    multivariateaccumulator.cpp \
    normaldistribution.cpp \
	primenumbers.cpp \
    statistics.cpp \
    symmetricschurdecomposition.cpp

noinst_LTLIBRARIES = libMath.la
