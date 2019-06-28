
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    armijo.hpp \
    bfgs.hpp \
    conjugategradient.hpp \
    constraint.hpp \
    costfunction.hpp \
    differentialevolution.hpp \
    endcriteria.hpp \
    goldstein.hpp \
    leastsquare.hpp \
    levenbergmarquardt.hpp \
    linesearch.hpp \
    linesearchbasedmethod.hpp \
    lmdif.hpp \
    method.hpp \
    problem.hpp \
    projectedconstraint.hpp \
    projectedcostfunction.hpp \
    projection.hpp \
    simplex.hpp \
    simulatedannealing.hpp \
    spherecylinder.hpp \
    steepestdescent.hpp

cpp_files = \
    armijo.cpp \
    bfgs.cpp \
    conjugategradient.cpp \
    constraint.cpp \
    differentialevolution.cpp \
    endcriteria.cpp \
    goldstein.cpp \
    leastsquare.cpp \
    levenbergmarquardt.cpp \
    linesearch.cpp \
    linesearchbasedmethod.cpp \
    lmdif.cpp \
    projectedcostfunction.cpp \
    projection.cpp \
    simplex.cpp \
    spherecylinder.cpp \
    steepestdescent.cpp

if UNITY_BUILD

nodist_libOptimization_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libOptimization_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libOptimization.la

include ${top_srcdir}/mk/all.mk

