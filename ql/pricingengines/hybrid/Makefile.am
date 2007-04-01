
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/pricingengines/hybrid

include_HEADERS = \
	all.hpp \
	binomialconvertibleengine.hpp \
	discretizedconvertible.hpp

libHybridEngines_la_SOURCES = \
    discretizedconvertible.cpp

noinst_LTLIBRARIES = libHybridEngines.la

