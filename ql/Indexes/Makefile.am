
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Indexes

include_HEADERS = \
    euribor.hpp \
    gbplibor.hpp \
    usdlibor.hpp \
    xibor.hpp \
    xibormanager.hpp

EXTRA_DIST=makefile.mak

libIndexes_la_SOURCES = \
    xibor.cpp \
    xibormanager.cpp

noinst_LTLIBRARIES = libIndexes.la
