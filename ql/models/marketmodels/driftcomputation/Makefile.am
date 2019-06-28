
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	cmsmmdriftcalculator.hpp \
	lmmdriftcalculator.hpp \
	lmmnormaldriftcalculator.hpp \
	smmdriftcalculator.hpp

cpp_files = \
	cmsmmdriftcalculator.cpp \
	lmmdriftcalculator.cpp \
	lmmnormaldriftcalculator.cpp \
	smmdriftcalculator.cpp

if UNITY_BUILD

nodist_libMarketModelsDriftComputation_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsDriftComputation_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsDriftComputation.la

include ${top_srcdir}/mk/all.mk

