
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
	avgpriceasianpathpricer.hpp \
	avgstrikeasianpathpricer.hpp \
	basketpathpricer.hpp \
	controlvariatedpathpricer.hpp \
	europeanpathpricer.hpp \
	everestpathpricer.hpp \
	geometricasianpathpricer.hpp \
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
	avgpriceasianpathpricer.cpp \
	avgstrikeasianpathpricer.cpp \
	basketpathpricer.cpp    \
	controlvariatedpathpricer.cpp \
	europeanpathpricer.cpp \
	everestpathpricer.cpp \
	geometricasianpathpricer.cpp \
	getcovariance.cpp \
	himalayapathpricer.cpp \
	pagodapathpricer.cpp \
	singleassetpathpricer.cpp

noinst_LTLIBRARIES = libMonteCarlo.la
