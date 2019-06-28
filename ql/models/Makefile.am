
SUBDIRS = equity marketmodels shortrate volatility

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	calibrationhelper.hpp \
	model.hpp \
	parameter.hpp

cpp_files = \
	calibrationhelper.cpp \
	model.cpp

if UNITY_BUILD

nodist_libModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libModels_la_SOURCES = $(cpp_files)

endif

libModels_la_LIBADD = \
    equity/libEquityModels.la \
    marketmodels/libMarketModels.la \
    shortrate/libShortRateModels.la \
    volatility/libVolatilityModels.la

noinst_LTLIBRARIES = libModels.la

include ${top_srcdir}/mk/all.mk

