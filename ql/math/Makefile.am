
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/math

SUBDIRS = optimization randomnumbers \
	interpolations distributions integrals solvers1d

include_HEADERS = \
	all.hpp \
	core.hpp \
	array.hpp \
	beta.hpp \
	choleskydecomposition.hpp \
	comparison.hpp \
	convergencestatistics.hpp \
	curve.hpp \
	discrepancystatistics.hpp \
	domain.hpp \
	errorfunction.hpp \
	factorial.hpp \
	functional.hpp \
	gaussianstatistics.hpp \
	generalstatistics.hpp \
	incompletegamma.hpp \
	incrementalstatistics.hpp \
	interpolation.hpp \
	lexicographicalview.hpp \
	linearleastsquaresregression.hpp \
	matrix.hpp \
	primenumbers.hpp \
	pseudosqrt.hpp \
	riskstatistics.hpp \
	rounding.hpp \
	sampledcurve.hpp \
	sequencestatistics.hpp \
	statistics.hpp \
	surface.hpp \
	svd.hpp \
	symmetricschurdecomposition.hpp \
	tqreigendecomposition.hpp \
	transformedgrid.hpp 

libMath_la_SOURCES = \
	beta.cpp \
	choleskydecomposition.cpp \
	discrepancystatistics.cpp \
	errorfunction.cpp \
	factorial.cpp \
	generalstatistics.cpp \
	incompletegamma.cpp \
	incrementalstatistics.cpp \
	matrix.cpp \
	primenumbers.cpp \
	pseudosqrt.cpp \
	rounding.cpp \
	sampledcurve.cpp \
	surface.cpp \
	svd.cpp \
	symmetricschurdecomposition.cpp \
	tqreigendecomposition.cpp

libMath_la_LIBADD = \
    distributions/libDistributions.la \
    integrals/libIntegrals.la \
    optimization/libOptimization.la \
    randomnumbers/libRandomNumbers.la

noinst_LTLIBRARIES = libMath.la

