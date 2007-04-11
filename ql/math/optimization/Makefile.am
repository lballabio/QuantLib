
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    core.hpp \
    armijo.hpp \
    conjugategradient.hpp \
    constraint.hpp \
    costfunction.hpp \
    endcriteria.hpp \
    leastsquare.hpp \
    levenbergmarquardt.hpp \
    linesearch.hpp \
    linesearchbasedmethod.hpp \
    lmdif.hpp \
    method.hpp \
    problem.hpp \
    simplex.hpp \
    steepestdescent.hpp

libOptimization_la_SOURCES = \
    armijo.cpp \
    conjugategradient.cpp \
    constraint.cpp \
    endcriteria.cpp \
    leastsquare.cpp \
    levenbergmarquardt.cpp \
    linesearch.cpp \
    linesearchbasedmethod.cpp \
    lmdif.cpp \
    simplex.cpp \
    steepestdescent.cpp

noinst_LTLIBRARIES = libOptimization.la

