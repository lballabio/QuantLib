
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/FiniteDifferences

include_HEADERS = \
	all.hpp \
	core.hpp \
	americancondition.hpp \
	boundarycondition.hpp \
	bsmoperator.hpp \
	bsmtermoperator.hpp \
	cranknicolson.hpp \
	dminus.hpp \
	dplus.hpp \
	dplusdminus.hpp \
	dzero.hpp \
	expliciteuler.hpp \
	fdtypedefs.hpp \
	finitedifferencemodel.hpp \
	impliciteuler.hpp \
	mixedscheme.hpp\
    onefactoroperator.hpp \
    operatortraits.hpp \
    parallelevolver.hpp \
    shoutcondition.hpp \
	stepcondition.hpp \
	tridiagonaloperator.hpp \
	valueatcenter.hpp

EXTRA_DIST=makefile.mak

libFiniteDifferences_la_SOURCES = \
    boundarycondition.cpp \
    bsmoperator.cpp \
    bsmtermoperator.cpp \
    onefactoroperator.cpp \
    tridiagonaloperator.cpp \
    valueatcenter.cpp

noinst_LTLIBRARIES = libFiniteDifferences.la
