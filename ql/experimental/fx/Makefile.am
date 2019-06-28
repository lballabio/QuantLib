
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    blackdeltacalculator.hpp \
    deltavolquote.hpp

cpp_files = \
    blackdeltacalculator.cpp \
    deltavolquote.cpp

if UNITY_BUILD

nodist_libFX_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFX_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFX.la

include ${top_srcdir}/mk/all.mk

