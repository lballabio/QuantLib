
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	analyticcliquetengine.hpp \
	analyticperformanceengine.hpp \
	mcperformanceengine.hpp

cpp_files = \
    analyticcliquetengine.cpp \
    analyticperformanceengine.cpp \
    mcperformanceengine.cpp

if UNITY_BUILD

nodist_libCliquetEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCliquetEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCliquetEngines.la

include ${top_srcdir}/mk/all.mk

