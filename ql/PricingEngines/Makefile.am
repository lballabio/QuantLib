
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

include_HEADERS = \
   discretizedvanillaoption.hpp \
   genericengine.hpp \
   forwardengines.hpp \
   latticeshortratemodelengine.hpp \
   mcengine.hpp \
   quantoengines.hpp \
   vanillaengines.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
    analyticeuropeanengine.cpp \
    binomialvanillaengine.cpp \
    discretizedvanillaoption.cpp \
    fdvanillaengine.cpp

noinst_LTLIBRARIES = libPricingEngines.la
