
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
    arithmeticapopathpricer.hpp \
    arithmeticasopathpricer.hpp \
	basketpathpricer.hpp \
    cliquetoptionpathpricer.hpp \
	europeanpathpricer.hpp \
	everestpathpricer.hpp \
	geometricapopathpricer.hpp \
	geometricasopathpricer.hpp \
	getcovariance.hpp \
	himalayapathpricer.hpp \
	mctypedefs.hpp \
	maxbasketpathpricer.hpp \
	montecarlomodel.hpp \
	multipath.hpp \
	multipathgenerator.hpp \
	pagodapathpricer.hpp \
	path.hpp \
	pathgenerator.hpp \
	pathpricer.hpp \
	performanceoptionpathpricer.hpp \
	sample.hpp


EXTRA_DIST=makefile.mak

libMonteCarlo_la_SOURCES = \
    arithmeticapopathpricer.cpp \
    arithmeticasopathpricer.cpp \
	basketpathpricer.cpp    \
    cliquetoptionpathpricer.cpp \
	europeanpathpricer.cpp \
	everestpathpricer.cpp \
	geometricapopathpricer.cpp \
	geometricasopathpricer.cpp \
	getcovariance.cpp \
	himalayapathpricer.cpp \
	maxbasketpathpricer.cpp \
	pagodapathpricer.cpp \
	performanceoptionpathpricer.cpp

noinst_LTLIBRARIES = libMonteCarlo.la
