
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

include_HEADERS = \
    americanmcengines.hpp \
    barrierengines.hpp \
    binaryengines.hpp \
    cliquetengines.hpp \
    discretizedvanillaoption.hpp \
    genericmodelengine.hpp \
    forwardengines.hpp \
    latticeshortratemodelengine.hpp \
    mcengine.hpp \
    mceuropeanengine.hpp \
    quantoengines.hpp \
    vanillaengines.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
    americanmcengines.cpp \
    analyticamericanbinaryengine.cpp \
    analyticbarrierengine.cpp \
    analyticeuropeanbinaryengine.cpp \
    analyticeuropeanengine.cpp \
    discretizedvanillaoption.cpp \
    integralengines.cpp

noinst_LTLIBRARIES = libPricingEngines.la
