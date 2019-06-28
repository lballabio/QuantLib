
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    claytoncopularng.hpp \
    convolvedstudentt.hpp \
    expm.hpp \
    farliegumbelmorgensterncopularng.hpp \
    fireflyalgorithm.hpp \
    frankcopularng.hpp \
    gaussiancopulapolicy.hpp \
    gaussiannoncentralchisquaredpolynomial.hpp \
    hybridsimulatedannealing.hpp \
    hybridsimulatedannealingfunctors.hpp \
    isotropicrandomwalk.hpp \
    laplaceinterpolation.hpp \
    latentmodel.hpp \
    levyflightdistribution.hpp \
    moorepenroseinverse.hpp \
    multidimintegrator.hpp \
    multidimquadrature.hpp \
    particleswarmoptimization.hpp \
    piecewisefunction.hpp \
    piecewiseintegral.hpp \
    polarstudenttrng.hpp \
    tcopulapolicy.hpp \
    zigguratrng.hpp

cpp_files = \
    convolvedstudentt.cpp \
    expm.cpp \
    fireflyalgorithm.cpp \
    gaussiancopulapolicy.cpp \
    gaussiannoncentralchisquaredpolynomial.cpp \
    multidimintegrator.cpp \
    multidimquadrature.cpp \
    particleswarmoptimization.cpp \
    piecewiseintegral.cpp \
    tcopulapolicy.cpp \
    zigguratrng.cpp

if UNITY_BUILD

nodist_libMath_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMath_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMath.la

include ${top_srcdir}/mk/all.mk

