
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

include_HEADERS = \
    americanmcengines.hpp \
    binaryengines.hpp \
    discretizedvanillaoption.hpp \
    mcvanillaengine.hpp \
    mceuropeanengine.hpp \
    quantoengines.hpp \
    vanillaengines.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
    americanmcengines.cpp \
    analyticamericanbinaryengine.cpp \
    analyticeuropeanbinaryengine.cpp \
    analyticeuropeanengine.cpp \
    discretizedvanillaoption.cpp \
    integralengines.cpp

noinst_LTLIBRARIES = libPricingEngines.la
