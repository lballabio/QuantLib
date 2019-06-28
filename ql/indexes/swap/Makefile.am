
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    chfliborswap.hpp \
    euriborswap.hpp \
    eurliborswap.hpp \
    gbpliborswap.hpp \
    jpyliborswap.hpp \
    usdliborswap.hpp

cpp_files = \
    chfliborswap.cpp \
    euriborswap.cpp \
    eurliborswap.cpp \
    gbpliborswap.cpp \
    jpyliborswap.cpp \
    usdliborswap.cpp

if UNITY_BUILD

nodist_libSwapIndexes_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libSwapIndexes_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libSwapIndexes.la

include ${top_srcdir}/mk/all.mk

