
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}

includedir = $(prefix)/include/ql/Math
include_HEADERS = \
	matrix.hpp \
	normaldistribution.hpp \
	statistics.hpp \
	riskmeasures.hpp \
	cubicspline.hpp \
	interpolation.hpp \
	linearinterpolation.hpp \
	lexicographicalview.hpp \
	multivariateaccumulator.hpp \
	segmentintegral.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = normaldistribution.cpp \
    matrix.cpp \
    multivariateaccumulator.cpp \
    segmentintegral.cpp \
    statistics.cpp \
    symmetricschurdecomposition.cpp

noinst_LTLIBRARIES = libMath.la
