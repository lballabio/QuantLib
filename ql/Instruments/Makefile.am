
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
	plainoption.hpp \
	simpleswap.hpp \
	stock.hpp \
	swap.hpp

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
	plainoption.cpp \
	simpleswap.cpp \
	stock.cpp \
	swap.cpp

noinst_LTLIBRARIES = libInstruments.la
