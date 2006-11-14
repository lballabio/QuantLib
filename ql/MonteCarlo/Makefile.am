
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
	all.hpp \
	core.hpp \
	brownianbridge.hpp \
	earlyexercisepathpricer.hpp \
	exercisestrategy.hpp \
	getcovariance.hpp \
	genericlsregression.hpp \
	longstaffschwartzpathpricer.hpp \
	lsmbasissystem.hpp \
	mctypedefs.hpp \
	mctraits.hpp \
	montecarlomodel.hpp \
	multipath.hpp \
	multipathgenerator.hpp \
	nodedata.hpp \
	path.hpp \
	pathgenerator.hpp \
	pathpricer.hpp \
	sample.hpp

libMonteCarlo_la_SOURCES = \
	brownianbridge.cpp \
	genericlsregression.cpp \
	getcovariance.cpp \
	lsmbasissystem.cpp

noinst_LTLIBRARIES = libMonteCarlo.la

EXTRA_DIST = makefile.mak

