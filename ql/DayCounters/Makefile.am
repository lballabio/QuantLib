
# $Id$
# $Source$
# $Log$
# Revision 1.2  2001/09/04 08:47:25  sadrejeb
# Small fix enabling QuantLib to build from a build/ directory
#
# Revision 1.1  2001/09/03 14:00:57  nando
# source (*.hpp and *.cpp) moved under topdir/ql
#
# Revision 1.4  2001/08/29 18:12:59  nando
# added 2 more ActualActual day count convention
# not finished yet
#
# Revision 1.3  2001/05/24 12:52:01  nando
# smoothing #include xx.hpp
#

INCLUDES= -I${top_srcdir} -I${top_builddir}

includedir = $(prefix)/include/ql/DayCounters
include_HEADERS = actual360.hpp \
	actual365.hpp \
	actualactual.hpp \
	actualactualeuro.hpp \
	actualactualhistorical.hpp \
	thirty360.hpp \
	thirty360european.hpp \
	thirty360italian.hpp

EXTRA_DIST=makefile.mak

libDayCounters_la_SOURCES = \
    actualactual.cpp \
    actualactualeuro.cpp \
    actualactualhistorical.cpp \
    thirty360.cpp \
    thirty360italian.cpp

#lib_LTLIBRARIES = libfoo.la
noinst_LTLIBRARIES = libDayCounters.la
