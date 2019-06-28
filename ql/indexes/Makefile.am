
SUBDIRS = ibor inflation swap

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    bmaindex.hpp \
    iborindex.hpp \
    indexmanager.hpp \
    inflationindex.hpp \
    interestrateindex.hpp \
    region.hpp \
    swapindex.hpp

cpp_files = \
    bmaindex.cpp \
    iborindex.cpp \
    indexmanager.cpp \
    inflationindex.cpp \
    interestrateindex.cpp \
    region.cpp \
    swapindex.cpp

libIndexes_la_LIBADD = \
    ibor/libIborIndexes.la \
    swap/libSwapIndexes.la

if UNITY_BUILD

nodist_libIndexes_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libIndexes_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libIndexes.la

include ${top_srcdir}/mk/all.mk

