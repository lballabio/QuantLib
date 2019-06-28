
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	cotswapfromfwdcorrelation.hpp \
	expcorrelations.hpp \
	timehomogeneousforwardcorrelation.hpp

cpp_files = \
	cotswapfromfwdcorrelation.cpp \
	expcorrelations.cpp \
	timehomogeneousforwardcorrelation.cpp

if UNITY_BUILD

nodist_libMarketModelsCorrelations_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsCorrelations_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsCorrelations.la

include ${top_srcdir}/mk/all.mk

