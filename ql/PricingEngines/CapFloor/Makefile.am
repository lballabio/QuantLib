
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/CapFloor

include_HEADERS = \
    all.hpp \
    analyticalcapfloor.hpp \
    blackcapfloor.hpp \
    capfloorpricer.hpp \
    treecapfloor.hpp

EXTRA_DIST=makefile.mak

libCapFloorEngines_la_SOURCES = \
    analyticalcapfloor.cpp \
    blackcapfloor.cpp \
    capfloorpricer.cpp \
    treecapfloor.cpp

noinst_LTLIBRARIES = libCapFloorEngines.la
