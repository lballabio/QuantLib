
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
	antitheticpathgenerator.hpp \
	avgpriceasianpathpricer.hpp \
	avgstrikeasianpathpricer.hpp \
	basketpathpricer.hpp \
	boxmuller.hpp \
	centrallimitgaussian.hpp \
	controlvariatedpathpricer.hpp \
	europeanpathpricer.hpp \
	everestpathpricer.hpp \
	gaussianmultipathgenerator.hpp \
	gaussianpathgenerator.hpp \
	gaussianrandomgenerator.hpp \
	generalmontecarlo.hpp \
	geometricasianpathpricer.hpp \
	getcovariance.hpp \
	himalayapathpricer.hpp \
	inversecumulativegaussian.hpp \
	knuthrandomgenerator.hpp \
	lecuyerrandomgenerator.hpp \
	mcoptionsample.hpp \
	mcpricer.hpp \
	montecarlomodel.hpp \
	multifactormontecarlooption.hpp \
	multifactorpricer.hpp \
	multipath.hpp \
	multipathgenerator.hpp \
	multipathpricer.hpp \
	onefactormontecarlooption.hpp \
	pagodapathpricer.hpp \
	path.hpp \
	pathpricer.hpp \
	randomarraygenerator.hpp \
	uniformrandomgenerator.hpp

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
	knuthrandomgenerator.cpp \
	lecuyerrandomgenerator.cpp \
	pagodapathpricer.cpp

noinst_LTLIBRARIES = libMonteCarlo.la
