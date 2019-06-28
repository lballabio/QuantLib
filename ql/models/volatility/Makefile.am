
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    constantestimator.hpp \
    simplelocalestimator.hpp \
    garmanklass.hpp \
    garch.hpp

cpp_files = \
    constantestimator.cpp \
    garch.cpp

if UNITY_BUILD

nodist_libVolatilityModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libVolatilityModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libVolatilityModels.la

include ${top_srcdir}/mk/all.mk

