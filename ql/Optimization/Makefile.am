
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Optimization

include_HEADERS = \
    armijo.hpp \
    conjugategradient.hpp \
    constraint.hpp \
    costfunction.hpp \
    criteria.hpp \
    leastsquare.hpp \
    linesearch.hpp \
    method.hpp \
    problem.hpp \
    simplex.hpp \
    steepestdescent.hpp

EXTRA_DIST=makefile.mak

libOptimization_la_SOURCES = \
    armijo.cpp \
    conjugategradient.cpp \
    simplex.cpp \
    steepestdescent.cpp

noinst_LTLIBRARIES = libOptimization.la
