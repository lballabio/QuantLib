
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

include_HEADERS = \
   discretizedvanillaoption.hpp \
   genericengine.hpp \
   forwardengines.hpp \
   latticeshortratemodelengine.hpp \
   quantoengines.hpp \
   vanillaengines.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
    discretizedvanillaoption.cpp \
    europeananalyticalengine.cpp \
    europeanbinomialengine.cpp \
    europeanFDengine.cpp \
    europeanMCengine.cpp

noinst_LTLIBRARIES = libPricingEngines.la
