
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    binomialconvertibleengine.hpp \
    convertiblebond.hpp \
    discretizedconvertible.hpp \
    tflattice.hpp

cpp_files = \
    convertiblebond.cpp \
    discretizedconvertible.cpp

if UNITY_BUILD

nodist_libConvertibleBonds_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libConvertibleBonds_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libConvertibleBonds.la

include ${top_srcdir}/mk/all.mk

