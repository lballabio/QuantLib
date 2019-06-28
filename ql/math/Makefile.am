
SUBDIRS = copulas distributions integrals interpolations \
          matrixutilities ode optimization randomnumbers solvers1d \
          statistics

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	abcdmathfunction.hpp \
	all.hpp \
	array.hpp \
	autocovariance.hpp \
	bernsteinpolynomial.hpp \
	beta.hpp \
	bspline.hpp \
	comparison.hpp \
	curve.hpp \
	errorfunction.hpp \
	factorial.hpp \
	fastfouriertransform.hpp \
	functional.hpp \
	generallinearleastsquares.hpp \
	kernelfunctions.hpp \
	incompletegamma.hpp \
	initializers.hpp \
	interpolation.hpp \
	lexicographicalview.hpp \
	linearleastsquaresregression.hpp \
	matrix.hpp \
	modifiedbessel.hpp \
	pascaltriangle.hpp \
	polynomialmathfunction.hpp \
	primenumbers.hpp \
	quadratic.hpp \
	rounding.hpp \
	richardsonextrapolation.hpp \
	sampledcurve.hpp \
	solver1d.hpp \
	transformedgrid.hpp

cpp_files = \
	abcdmathfunction.cpp \
	bernsteinpolynomial.cpp \
	beta.cpp \
	bspline.cpp \
	errorfunction.cpp \
	factorial.cpp \
	incompletegamma.cpp \
	matrix.cpp \
	modifiedbessel.cpp \
	pascaltriangle.cpp \
	polynomialmathfunction.cpp \
	primenumbers.cpp \
	quadratic.cpp \
	richardsonextrapolation.cpp \
	rounding.cpp \
	sampledcurve.cpp

if UNITY_BUILD

nodist_libMath_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMath_la_SOURCES = $(cpp_files)

endif

libMath_la_LIBADD = \
    copulas/libCopulas.la \
    distributions/libDistributions.la \
    integrals/libIntegrals.la \
    matrixutilities/libMatrixUtilities.la \
    optimization/libOptimization.la \
    randomnumbers/libRandomNumbers.la \
    statistics/libStatistics.la

noinst_LTLIBRARIES = libMath.la

include ${top_srcdir}/mk/all.mk
