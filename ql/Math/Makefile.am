
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Math

include_HEADERS = \
	bilinearinterpolation.hpp \
    chisquaredistribution.hpp \
	cubicspline.hpp \
	interpolation.hpp \
	interpolation2D.hpp \
    gammadistribution.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	multivariateaccumulator.hpp \
	normaldistribution.hpp \
	riskmeasures.hpp \
	segmentintegral.hpp \
	statistics.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
    chisquaredistribution.cpp \
    gammadistribution.cpp \
    matrix.cpp \
    multivariateaccumulator.cpp \
    normaldistribution.cpp \
    statistics.cpp \
    symmetricschurdecomposition.cpp

noinst_LTLIBRARIES = libMath.la
