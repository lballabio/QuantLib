
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Math

include_HEADERS = \
	beta.hpp \
	bicubicsplineinterpolation.hpp \
	bilinearinterpolation.hpp \
	chisquaredistribution.hpp \
	comparison.hpp \
	cubicspline.hpp \
	discrepancystatistics.hpp \
	errorfunction.hpp \
	factorial.hpp \
	functional.hpp \
	gammadistribution.hpp \
	gaussianstatistics.hpp \
	generalstatistics.hpp \
	incompletegamma.hpp \
	incrementalstatistics.hpp \
	interpolation.hpp \
	interpolation2D.hpp \
	interpolationtraits.hpp \
	kronrodintegral.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	normaldistribution.hpp \
	poissondistribution.hpp \
	primenumbers.hpp \
	riskstatistics.hpp \
	segmentintegral.hpp \
	sequencestatistics.hpp \
	simpsonintegral.hpp \
	statistics.hpp \
	svd.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp \
	trapezoidintegral.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
	beta.cpp \
	chisquaredistribution.cpp \
	discrepancystatistics.cpp \
	errorfunction.cpp \
	factorial.cpp \
	gammadistribution.cpp \
	generalstatistics.cpp \
	incompletegamma.cpp \
	incrementalstatistics.cpp \
	matrix.cpp \
	normaldistribution.cpp \
	primenumbers.cpp \
	svd.cpp \
	symmetricschurdecomposition.cpp


noinst_LTLIBRARIES = libMath.la
