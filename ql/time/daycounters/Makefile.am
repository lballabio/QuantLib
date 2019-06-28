
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	actual360.hpp \
	actual365fixed.hpp \
	actualactual.hpp \
	business252.hpp \
	one.hpp \
	simpledaycounter.hpp \
	thirty360.hpp

cpp_files = \
    actual365fixed.cpp \
    actualactual.cpp \
    business252.cpp \
	simpledaycounter.cpp \
    thirty360.cpp

if UNITY_BUILD

nodist_libDayCounters_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libDayCounters_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libDayCounters.la

include ${top_srcdir}/mk/all.mk

