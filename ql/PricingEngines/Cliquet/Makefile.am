
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Cliquet

include_HEADERS = \
	all.hpp \
	analyticcliquetengine.hpp

EXTRA_DIST=makefile.mak

libCliquetEngines_la_SOURCES = \
    analyticcliquetengine.cpp

noinst_LTLIBRARIES = libCliquetEngines.la
