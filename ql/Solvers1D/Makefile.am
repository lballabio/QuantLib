
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}

includedir = $(prefix)/include/ql/Solvers1D
include_HEADERS = bisection.hpp \
	brent.hpp \
	falseposition.hpp \
	newton.hpp \
	newtonsafe.hpp \
	ridder.hpp \
	secant.hpp
EXTRA_DIST=makefile.mak

libSolvers1D_la_SOURCES = bisection.cpp \
	brent.cpp \
    falseposition.cpp \
	newton.cpp \
    newtonsafe.cpp \
	ridder.cpp \
	secant.cpp

noinst_LTLIBRARIES = libSolvers1D.la
