
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Vanilla

include_HEADERS = \
    americanmcengines.hpp \
    discretizedvanillaoption.hpp \
    mcvanillaengine.hpp \
    mceuropeanengine.hpp \
    vanillaengines.hpp

EXTRA_DIST=makefile.mak

libVanillaEngines_la_SOURCES = \
    americanmcengines.cpp \
    analyticeuropeanengine.cpp \
    discretizedvanillaoption.cpp \
    integralengines.cpp

noinst_LTLIBRARIES = libVanillaEngines.la
