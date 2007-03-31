
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/BrownianGenerators

include_HEADERS = \
	all.hpp \
	mtbrowniangenerator.hpp \
	sobolbrowniangenerator.hpp

libMarketModelsBrownianGenerators_la_SOURCES = \
	mtbrowniangenerator.cpp \
	sobolbrowniangenerator.cpp

noinst_LTLIBRARIES = libMarketModelsBrownianGenerators.la

