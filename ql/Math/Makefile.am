
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
	factorial.hpp \
	gammadistribution.hpp \
	gaussianstatistics.hpp \
	generalstatistics.hpp \
	incrementalstatistics.hpp \
	kronrodintegral.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	normaldistribution.hpp \
	primenumbers.hpp \
	riskstatistics.hpp \
	segmentintegral.hpp \
	sequencestatistics.hpp \
	simpsonintegral.hpp \
	statistics.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp \
	svd.hpp \
	trapezoidintegral.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
	chisquaredistribution.cpp \
	discrepancystatistics.cpp \
	errorfunction.cpp \
	factorial.cpp \
	gammadistribution.cpp \
	generalstatistics.cpp \
	incrementalstatistics.cpp \
	matrix.cpp \
	normaldistribution.cpp \
	primenumbers.cpp \
	symmetricschurdecomposition.cpp \
	svd.cpp

noinst_LTLIBRARIES = libMath.la
