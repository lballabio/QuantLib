
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
    capfloor.hpp \
    quantooption.hpp \
    simpleswap.hpp \
    stock.hpp \
    swap.hpp \
    swaption.hpp \
    vanillaoption.hpp

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
    capfloor.cpp \
    quantooption.cpp \
    simpleswap.cpp \
    stock.cpp \
    swap.cpp \
    swaption.cpp \
    vanillaoption.cpp

noinst_LTLIBRARIES = libInstruments.la
