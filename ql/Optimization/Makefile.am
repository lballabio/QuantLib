
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Optimization

include_HEADERS = \
    all.hpp \
    core.hpp \
    armijo.hpp \
    conjugategradient.hpp \
    constraint.hpp \
    costfunction.hpp \
    criteria.hpp \
    leastsquare.hpp \
    levenbergmarquardt.hpp \
    linesearch.hpp \
    lmdif.hpp \
    method.hpp \
    problem.hpp \
    simplex.hpp \
    steepestdescent.hpp

libOptimization_la_SOURCES = \
    armijo.cpp \
    conjugategradient.cpp \
    levenbergmarquardt.cpp \
    lmdif.cpp \
    simplex.cpp \
    steepestdescent.cpp

noinst_LTLIBRARIES = libOptimization.la

EXTRA_DIST = makefile.mak

