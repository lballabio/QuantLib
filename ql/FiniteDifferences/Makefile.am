
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}

includedir = $(prefix)/include/ql/FiniteDifferences
include_HEADERS = backwardeuler.hpp \
	boundarycondition.hpp \
	bsmoperator.hpp \
	cranknicolson.hpp \
	dminus.hpp \
	dplus.hpp \
	dplusdminus.hpp \
	dzero.hpp \
	finitedifferencemodel.hpp \
	forwardeuler.hpp \
	identity.hpp \
	operator.hpp \
	operatortraits.hpp \
	stepcondition.hpp \
	standardstepcondition.hpp \
	standardfdmodel.hpp \
	tridiagonaloperator.hpp \
	valueatcenter.hpp

EXTRA_DIST=makefile.mak

libFiniteDifferences_la_SOURCES = \
    bsmoperator.cpp \
    tridiagonaloperator.cpp \
    valueatcenter.cpp

noinst_LTLIBRARIES = libFiniteDifferences.la
