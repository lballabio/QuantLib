
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
	all.hpp \
	core.hpp \
	arithmeticapopathpricer.hpp \
	arithmeticasopathpricer.hpp \
	barrierpathpricer.hpp \
	basketpathpricer.hpp \
	biasedbarrierpathpricer.hpp \
	binarybarrierpathpricer.hpp \
	brownianbridge.hpp \
	cliquetoptionpathpricer.hpp \
	digitalpathpricer.hpp \
	europeanmultipathpricer.hpp \
	europeanpathpricer.hpp \
	everestpathpricer.hpp \
	geometricapopathpricer.hpp \
	geometricasopathpricer.hpp \
	getcovariance.hpp \
	himalayapathpricer.hpp \
	mctypedefs.hpp \
	maxbasketpathpricer.hpp \
	mctraits.hpp \
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
	barrierpathpricer.cpp \
	basketpathpricer.cpp \
	biasedbarrierpathpricer.cpp \
	binarybarrierpathpricer.cpp \
	cliquetoptionpathpricer.cpp \
	digitalpathpricer.cpp \
	europeanmultipathpricer.cpp \
	europeanpathpricer.cpp \
	everestpathpricer.cpp \
	geometricapopathpricer.cpp \
	geometricasopathpricer.cpp \
	himalayapathpricer.cpp \
	maxbasketpathpricer.cpp \
	pagodapathpricer.cpp \
	performanceoptionpathpricer.cpp

noinst_LTLIBRARIES = libMonteCarlo.la
