# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Optimization

# SUBDIRS = GeneticAlgorithms

include_HEADERS = \
    armijo.hpp \
    conjugategradient.hpp \
    constraint.hpp \
    costfunction.hpp \
    criteria.hpp \
    leastsquare.hpp \
    linesearch.hpp \
    optimizer.hpp \
    simplex.hpp \
    steepestdescent.hpp

EXTRA_DIST=makefile.mak

libOptimization_la_SOURCES = \
    armijo.cpp \
    conjugategradient.cpp \
    simplex.cpp \
    steepestdescent.cpp

noinst_LTLIBRARIES = libOptimization.la
