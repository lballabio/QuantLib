
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}

includedir = $(prefix)/include/ql/Instruments
include_HEADERS = stock.hpp \
                  simpleswap.hpp \
                  swap.hpp

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
    swap.cpp \
    simpleswap.cpp \
    stock.cpp

noinst_LTLIBRARIES = libInstruments.la
