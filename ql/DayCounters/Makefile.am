
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}

include_HEADERS = \
	actual360.hpp \
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

noinst_LTLIBRARIES = libDayCounters.la
