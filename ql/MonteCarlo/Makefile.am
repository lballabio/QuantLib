
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MonteCarlo

include_HEADERS = \
	all.hpp \
	core.hpp \
	brownianbridge.hpp \
	getcovariance.hpp \
	mctypedefs.hpp \
	mctraits.hpp \
	montecarlomodel.hpp \
	multipath.hpp \
	multipathgenerator.hpp \
	path.hpp \
	pathgenerator.hpp \
	pathpricer.hpp \
	sample.hpp

libMonteCarlo_la_SOURCES = \
	getcovariance.cpp

noinst_LTLIBRARIES = libMonteCarlo.la
