
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Evolvers

include_HEADERS = \
	all.hpp \
	forwardrateconstrainedeuler.hpp \
	forwardrateeulerevolver.hpp \
	forwardrateipcevolver.hpp \
	forwardratepcevolver.hpp

libMarketModelsEvolvers_la_SOURCES = \
	forwardrateconstrainedeuler.cpp \
	forwardrateeulerevolver.cpp \
	forwardrateipcevolver.cpp \
	forwardratepcevolver.cpp

noinst_LTLIBRARIES = libMarketModelsEvolvers.la

