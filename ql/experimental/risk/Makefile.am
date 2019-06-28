
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    creditriskplus.hpp \
    sensitivityanalysis.hpp

cpp_files = \
    creditriskplus.cpp \
    sensitivityanalysis.cpp

if UNITY_BUILD

nodist_libRisk_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libRisk_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libRisk.la

include ${top_srcdir}/mk/all.mk

