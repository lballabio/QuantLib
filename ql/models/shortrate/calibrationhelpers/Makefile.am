
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    caphelper.hpp \
    swaptionhelper.hpp

cpp_files = \
    caphelper.cpp \
    swaptionhelper.cpp

if UNITY_BUILD

nodist_libCalibrationHelpers_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCalibrationHelpers_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCalibrationHelpers.la

include ${top_srcdir}/mk/all.mk

