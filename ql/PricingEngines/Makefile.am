
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

include_HEADERS = \
    americanmcengines.hpp \
    barrierengines.hpp \
    cliquetengines.hpp \
    discretizedvanillaoption.hpp \
    genericengine.hpp \
    forwardengines.hpp \
    latticeshortratemodelengine.hpp \
    mcengine.hpp \
    quantoengines.hpp \
    vanillaengines.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
    americanmcengines.cpp \
    analyticbarrierengine.cpp \
    analyticeuropeanengine.cpp \
    binomialvanillaengine.cpp \
    discretizedvanillaoption.cpp \
    integralengines.cpp \
    mcbarrierengine.cpp

noinst_LTLIBRARIES = libPricingEngines.la
