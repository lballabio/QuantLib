
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
    arithmeticapopathpricer.hpp \
    arithmeticasopathpricer.hpp \
	basketpathpricer.hpp \
	controlvariatedpathpricer.hpp \
	europeanpathpricer.hpp \
	everestpathpricer.hpp \
	geometricapopathpricer.hpp \
	geometricasopathpricer.hpp \
	getcovariance.hpp \
	himalayapathpricer.hpp \
	mctypedefs.hpp \
	montecarlomodel.hpp \
	multipath.hpp \
	multipathgenerator.hpp \
	multipathpricer.hpp \
	pagodapathpricer.hpp \
	path.hpp \
	pathgenerator.hpp \
	pathpricer.hpp \
	sample.hpp \
	singleassetpathpricer.hpp

EXTRA_DIST=makefile.mak

libMonteCarlo_la_SOURCES = \
    arithmeticapopathpricer.cpp \
    arithmeticasopathpricer.cpp \
	basketpathpricer.cpp    \
	controlvariatedpathpricer.cpp \
	europeanpathpricer.cpp \
	everestpathpricer.cpp \
	geometricapopathpricer.cpp \
	geometricasopathpricer.cpp \
	getcovariance.cpp \
	himalayapathpricer.cpp \
	pagodapathpricer.cpp \
	singleassetpathpricer.cpp

noinst_LTLIBRARIES = libMonteCarlo.la
