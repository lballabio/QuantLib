
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Vanilla

include_HEADERS = \
    americanmcengines.hpp \
    discretizedvanillaoption.hpp \
    jumpdiffusionengine.hpp \
    mcdigitalengine.hpp \
    mceuropeanengine.hpp \
    mcvanillaengine.hpp \
    vanillaengines.hpp

EXTRA_DIST=makefile.mak

libVanillaEngines_la_SOURCES = \
    americanmcengines.cpp \
    analyticamericanengine.cpp \
    analyticeuropeanengine.cpp \
    baroneadesiwhaleyengine.cpp \
    bjerksundstenslandengine.cpp \
    discretizedvanillaoption.cpp \
    integralengines.cpp \
    jumpdiffusionengine.cpp

noinst_LTLIBRARIES = libVanillaEngines.la
