
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/functions

include_HEADERS = \
	daycounters.hpp \
	mathf.hpp \
	vols.hpp

EXTRA_DIST=makefile.mak

libfunctions_la_SOURCES = \
    daycounters.cpp \
    mathf.cpp \
	vols.cpp

noinst_LTLIBRARIES = libfunctions.la
