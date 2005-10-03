
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Cliquet

include_HEADERS = \
	all.hpp \
	analyticcliquetengine.hpp \
	analyticperformanceengine.hpp

libCliquetEngines_la_SOURCES = \
    analyticcliquetengine.cpp \
    analyticperformanceengine.cpp

noinst_LTLIBRARIES = libCliquetEngines.la

EXTRA_DIST = makefile.mak

