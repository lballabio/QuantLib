
SUBDIRS = calendars daycounters

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    asx.hpp \
    businessdayconvention.hpp \
    calendar.hpp \
    date.hpp \
    dategenerationrule.hpp \
	daycounter.hpp \
	ecb.hpp \
    frequency.hpp \
    imm.hpp \
    period.hpp \
    schedule.hpp \
    timeunit.hpp \
    weekday.hpp

cpp_files = \
    asx.cpp \
    businessdayconvention.cpp \
    calendar.cpp \
    date.cpp \
    dategenerationrule.cpp \
    ecb.cpp \
    frequency.cpp \
    imm.cpp \
    period.cpp \
    schedule.cpp \
    timeunit.cpp \
    weekday.cpp

if UNITY_BUILD

nodist_libTime_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libTime_la_SOURCES = $(cpp_files)

endif

libTime_la_LIBADD = \
    calendars/libCalendars.la \
    daycounters/libDayCounters.la

noinst_LTLIBRARIES = libTime.la

include ${top_srcdir}/mk/all.mk

