
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
	simpleswap.hpp \
	stock.hpp \
	swap.hpp

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
    simpleswap.cpp \
    stock.cpp \
    swap.cpp

noinst_LTLIBRARIES = libInstruments.la
