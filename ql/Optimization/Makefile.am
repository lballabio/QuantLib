# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Optimization

include_HEADERS = \
    armijo.hpp \
    conjugategradient.hpp \
    costfunction.hpp \
    criteria.hpp \
    leastsquare.hpp \
    linesearch.hpp \
    steepestdescent.hpp

EXTRA_DIST=makefile.mak

libOptimization_la_SOURCES = \
    armijo.cpp \
    conjugategradient.cpp \
    steepestdescent.cpp

noinst_LTLIBRARIES = libOptimization.la
