
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    overnightindexfuture.hpp \
    overnightindexfutureratehelper.hpp

cpp_files = \
    overnightindexfuture.cpp \
    overnightindexfutureratehelper.cpp

if UNITY_BUILD

nodist_libFutures_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFutures_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFutures.la

include ${top_srcdir}/mk/all.mk

