
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
   quantoengine.hpp \
   discretizedvanillaoption.hpp \
   genericengine.hpp \
   vanillaengines.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    discretizedvanillaoption.cpp \
    europeanbinomialengine.cpp \
    europeananalyticalengine.cpp \
    quantoeuropeananalyticalengine.cpp

noinst_LTLIBRARIES = libPricers.la
