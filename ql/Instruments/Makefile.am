
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
    capfloor.hpp \
	plainoption.hpp \
	simpleswap.hpp \
	stock.hpp \
	swap.hpp

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
    capfloor.cpp \
    simpleswap.cpp \
    stock.cpp \
	plainoption.cpp \
	swap.cpp

noinst_LTLIBRARIES = libInstruments.la
