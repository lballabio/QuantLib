
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Math

include_HEADERS = \
	all.hpp \
	core.hpp \
	array.hpp \
	backwardflatinterpolation.hpp \
	beta.hpp \
	bicubicsplineinterpolation.hpp \
	bilinearinterpolation.hpp \
	binomialdistribution.hpp \
	bivariatenormaldistribution.hpp \
	chisquaredistribution.hpp \
	choleskydecomposition.hpp \
	comparison.hpp \
	cubicspline.hpp \
	discrepancystatistics.hpp \
	errorfunction.hpp \
	extrapolation.hpp \
	factorial.hpp \
	forwardflatinterpolation.hpp \
	functional.hpp \
	gammadistribution.hpp \
	gaussianorthogonalpolynomial.hpp \
	gaussianquadratures.hpp \
	gaussianstatistics.hpp \
	generalstatistics.hpp \
	incompletegamma.hpp \
	incrementalstatistics.hpp \
	interpolation.hpp \
	interpolation2D.hpp \
	kronrodintegral.hpp \
	lexicographicalview.hpp \
	linearinterpolation.hpp \
	loglinearinterpolation.hpp \
	matrix.hpp \
	multicubicspline.hpp \
	normaldistribution.hpp \
	poissondistribution.hpp \
	primenumbers.hpp \
	pseudosqrt.hpp \
	riskstatistics.hpp \
	rounding.hpp \
	segmentintegral.hpp \
	sequencestatistics.hpp \
	simpsonintegral.hpp \
	statistics.hpp \
	svd.hpp \
	symmetriceigenvalues.hpp \
	symmetricschurdecomposition.hpp \
	tqreigendecomposition.hpp \
	trapezoidintegral.hpp

EXTRA_DIST=makefile.mak

libMath_la_SOURCES = \
	beta.cpp \
	bivariatenormaldistribution.cpp \
	chisquaredistribution.cpp \
	choleskydecomposition.cpp \
	discrepancystatistics.cpp \
	errorfunction.cpp \
	factorial.cpp \
	gammadistribution.cpp \
	gaussianorthogonalpolynomial.cpp \
	gaussianquadratures.cpp \
	generalstatistics.cpp \
	incompletegamma.cpp \
	incrementalstatistics.cpp \
	normaldistribution.cpp \
	primenumbers.cpp \
	pseudosqrt.cpp \
	rounding.cpp \
	svd.cpp \
	symmetricschurdecomposition.cpp \
	tqreigendecomposition.cpp


noinst_LTLIBRARIES = libMath.la
