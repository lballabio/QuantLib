
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/time

SUBDIRS = calendars daycounters

include_HEADERS = \
    businessdayconvention.hpp \
    calendar.hpp \
    date.hpp \
    frequency.hpp \
    period.hpp \
    schedule.hpp \
    timeunit.hpp \
    weekday.hpp

libTime_la_SOURCES = \
    businessdayconvention.cpp \
    calendar.cpp \
    date.cpp \
    frequency.cpp \
    period.cpp \
    schedule.cpp \
    weekday.cpp

libTime_la_LIBADD = \
    calendars/libCalendars.la \
    daycounters/libDayCounters.la

noinst_LTLIBRARIES = libTime.la

