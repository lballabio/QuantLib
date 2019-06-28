
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    inflationcapfloorengines.hpp

cpp_files = \
    inflationcapfloorengines.cpp

if UNITY_BUILD

nodist_libInflationEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libInflationEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libInflationEngines.la

include ${top_srcdir}/mk/all.mk

