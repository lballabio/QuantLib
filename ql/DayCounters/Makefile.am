
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/DayCounters

include_HEADERS = \
	all.hpp \
	actual360.hpp \
	actual365fixed.hpp \
	actualactual.hpp \
	one.hpp \
	simpledaycounter.hpp \
	thirty360.hpp

EXTRA_DIST=makefile.mak

libDayCounters_la_SOURCES = \
    actualactual.cpp \
	simpledaycounter.cpp \
    thirty360.cpp

noinst_LTLIBRARIES = libDayCounters.la
