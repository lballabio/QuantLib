
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Math

include_HEADERS = \
	bilinearinterpolation.hpp \
	bicubicsplineinterpolation.hpp \
	chisquaredistribution.hpp \
	cubicspline.hpp \
	discrepancystatistics.hpp \
	errorfunction.hpp \
	functional.hpp \
	interpolation.hpp \
	interpolation2D.hpp \
	interpolationtraits.hpp \
	gammadistribution.hpp \
	gaussianstatistics.hpp \
	generalstatistics.hpp \
	incrementalstatistics.hpp \
	kronrodintegral.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	multivariateaccumulator.hpp \
	normaldistribution.hpp \
	primenumbers.hpp \
	riskmeasures.hpp \
	riskstatistics.hpp \
	segmentintegral.hpp \
	sequencestatistics.hpp \
	statistics.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp \
	svd.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
	chisquaredistribution.cpp \
	discrepancystatistics.cpp \
	errorfunction.cpp \
	gammadistribution.cpp \
	generalstatistics.cpp \
	incrementalstatistics.cpp \
	matrix.cpp \
	multivariateaccumulator.cpp \
	normaldistribution.cpp \
	primenumbers.cpp \
	symmetricschurdecomposition.cpp \
	svd.cpp

noinst_LTLIBRARIES = libMath.la
