
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Asian

include_HEADERS = \
	all.hpp \
	analyticcontinuousasianengine.hpp \
	analyticdiscreteasianengine.hpp

EXTRA_DIST=makefile.mak

libAsianEngines_la_SOURCES = \
    analyticcontinuousasianengine.cpp \
    analyticdiscreteasianengine.cpp

noinst_LTLIBRARIES = libAsianEngines.la

