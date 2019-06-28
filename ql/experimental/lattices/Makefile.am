
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    extendedbinomialtree.hpp

cpp_files = \
    extendedbinomialtree.cpp

if UNITY_BUILD

nodist_libLattices_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libLattices_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libLattices.la

include ${top_srcdir}/mk/all.mk

