
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	cmswapratepcevolver.hpp \
	coterminalswapratepcevolver.hpp \
	forwardrateconstrainedeuler.hpp \
	forwardrateeulerevolver.hpp \
	forwardrateipcevolver.hpp \
	forwardratenormalpcevolver.hpp \
	forwardratepcevolver.hpp

libMarketModelsEvolvers_la_SOURCES = \
	cmswapratepcevolver.cpp \
	coterminalswapratepcevolver.cpp \
	forwardrateconstrainedeuler.cpp \
	forwardrateeulerevolver.cpp \
	forwardrateipcevolver.cpp \
	forwardratenormalpcevolver.cpp \
	forwardratepcevolver.cpp

noinst_LTLIBRARIES = libMarketModelsEvolvers.la

