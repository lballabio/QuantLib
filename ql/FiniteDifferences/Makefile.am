
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/FiniteDifferences
include_HEADERS = \
	boundarycondition.hpp \
	bsmoperator.hpp \
	cranknicolson.hpp \
	dminus.hpp \
	dplus.hpp \
	dplusdminus.hpp \
	dzero.hpp \
	expliciteuler.hpp \
	fdtypedefs.hpp \
	finitedifferencemodel.hpp \
	impliciteuler.hpp \
    onefactoroperator.hpp \
	stepcondition.hpp \
	tridiagonaloperator.hpp \
	valueatcenter.hpp

EXTRA_DIST=makefile.mak

libFiniteDifferences_la_SOURCES = \
    bsmoperator.cpp \
    onefactoroperator.cpp \
    tridiagonaloperator.cpp \
    valueatcenter.cpp

noinst_LTLIBRARIES = libFiniteDifferences.la
