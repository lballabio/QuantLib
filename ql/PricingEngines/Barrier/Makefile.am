
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

include_HEADERS = \
    barrierengines.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
    analyticbarrierengine.cpp

noinst_LTLIBRARIES = libPricingEngines.la
