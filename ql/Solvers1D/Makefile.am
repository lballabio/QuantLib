
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Solvers1D

include_HEADERS = \
	bisection.hpp \
	brent.hpp \
	falseposition.hpp \
	newton.hpp \
	newtonsafe.hpp \
	ridder.hpp \
	secant.hpp


EXTRA_DIST=makefile.mak

libSolvers1D_la_SOURCES = \
	brent.cpp \
	newton.cpp \
	ridder.cpp \
	secant.cpp \
    bisection.cpp \
    falseposition.cpp \
    newtonsafe.cpp

noinst_LTLIBRARIES = libSolvers1D.la
