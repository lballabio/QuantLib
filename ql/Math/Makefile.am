
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}

include_HEADERS = \
	cubicspline.hpp \
	interpolation.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
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
    matrix.cpp \
    multivariateaccumulator.cpp \
    normaldistribution.cpp \
    segmentintegral.cpp \
    statistics.cpp \
    symmetricschurdecomposition.cpp

noinst_LTLIBRARIES = libMath.la
