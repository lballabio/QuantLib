
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    bondfunctions.hpp \
    discountingbondengine.hpp

cpp_files = \
    bondfunctions.cpp \
    discountingbondengine.cpp

if UNITY_BUILD

nodist_libBondEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libBondEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libBondEngines.la

include ${top_srcdir}/mk/all.mk

