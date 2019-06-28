
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	cmswapcurvestate.hpp \
	coterminalswapcurvestate.hpp \
	lmmcurvestate.hpp

cpp_files = \
	cmswapcurvestate.cpp \
	coterminalswapcurvestate.cpp \
	lmmcurvestate.cpp

if UNITY_BUILD

nodist_libMarketModelsCurveStates_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsCurveStates_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsCurveStates.la

include ${top_srcdir}/mk/all.mk

