
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Calendars
include_HEADERS = \
	frankfurt.hpp \
	helsinki.hpp \
    johannesburg.hpp\
	london.hpp \
	milan.hpp \
	newyork.hpp \
	target.hpp \
	tokyo.hpp \
	toronto.hpp \	
	wellington.hpp \
	zurich.hpp


EXTRA_DIST=makefile.mak

libCalendars_la_SOURCES = \
	frankfurt.cpp \
	helsinki.cpp \
    johannesburg.cpp\
	london.cpp \
	milan.cpp \
	newyork.cpp \
	target.cpp \
	tokyo.cpp \
	toronto.cpp \
	wellington.cpp \
	zurich.cpp

noinst_LTLIBRARIES = libCalendars.la
