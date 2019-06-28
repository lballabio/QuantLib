
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    catbond.hpp \
	catrisk.hpp \
	montecarlocatbondengine.hpp \
	riskynotional.hpp

cpp_files = \
    catbond.cpp \
	catrisk.cpp \
	montecarlocatbondengine.cpp \
	riskynotional.cpp

if UNITY_BUILD

nodist_libCatBonds_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCatBonds_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCatBonds.la

include ${top_srcdir}/mk/all.mk

