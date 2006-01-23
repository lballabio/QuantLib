
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Hybrid

include_HEADERS = \
	all.hpp \
	binomialconvertibleengine.hpp \
	discretizedconvertible.hpp

EXTRA_DIST=makefile.mak

libHybridEngines_la_SOURCES = \
    discretizedconvertible.cpp 

noinst_LTLIBRARIES = libHybridEngines.la

