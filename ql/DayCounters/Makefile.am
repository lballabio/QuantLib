
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/DayCounters
include_HEADERS = \
	actual360.hpp \
	actual365.hpp \
	actualactual.hpp \
	thirty360.hpp

EXTRA_DIST=makefile.mak

libDayCounters_la_SOURCES = \
    actualactual.cpp \
    thirty360.cpp

noinst_LTLIBRARIES = libDayCounters.la
