
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Evolvers

include_HEADERS = \
	forwardrateipcevolver.hpp \
	forwardratepcevolver.hpp

libMarketModelsEvolvers_la_SOURCES = \
	forwardrateipcevolver.cpp \
	forwardratepcevolver.cpp

noinst_LTLIBRARIES = libMarketModelsEvolvers.la

EXTRA_DIST = makefile.mak

