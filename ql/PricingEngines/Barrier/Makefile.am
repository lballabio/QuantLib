
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Barrier

include_HEADERS = \
    barrierengines.hpp

EXTRA_DIST=makefile.mak

libBarrierEngines_la_SOURCES = \
    analyticbarrierengine.cpp

noinst_LTLIBRARIES = libBarrierEngines.la
