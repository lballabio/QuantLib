
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/CapFloor

include_HEADERS = \
    all.hpp \
    analyticcapfloorengine.hpp \
    blackcapfloorengine.hpp \
    discretizedcapfloor.hpp \
    mchullwhiteengine.hpp \
    treecapfloorengine.hpp

libCapFloorEngines_la_SOURCES = \
    analyticcapfloorengine.cpp \
    blackcapfloorengine.cpp \
    discretizedcapfloor.cpp \
    mchullwhiteengine.cpp \
    treecapfloorengine.cpp

noinst_LTLIBRARIES = libCapFloorEngines.la

EXTRA_DIST = makefile.mak

