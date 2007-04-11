
SUBDIRS = libormarketmodels pricers

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp

libLegacy_la_SOURCES =

libLegacy_la_LIBADD = \
    libormarketmodels/libLiborMarketModels.la \
    pricers/libPricers.la

noinst_LTLIBRARIES = libLegacy.la

