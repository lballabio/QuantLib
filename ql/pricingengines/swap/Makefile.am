
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    cvaswapengine.hpp \
    discountingswapengine.hpp \
    discretizedswap.hpp \
    treeswapengine.hpp

cpp_files = \
    cvaswapengine.cpp \
    discountingswapengine.cpp \
    discretizedswap.cpp \
    treeswapengine.cpp

if UNITY_BUILD

nodist_libSwapEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libSwapEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libSwapEngines.la

include ${top_srcdir}/mk/all.mk

