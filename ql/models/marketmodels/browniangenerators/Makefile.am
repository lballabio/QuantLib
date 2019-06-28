
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	mtbrowniangenerator.hpp \
	sobolbrowniangenerator.hpp

cpp_files = \
	mtbrowniangenerator.cpp \
	sobolbrowniangenerator.cpp

if UNITY_BUILD

nodist_libMarketModelsBrownianGenerators_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsBrownianGenerators_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsBrownianGenerators.la

include ${top_srcdir}/mk/all.mk

