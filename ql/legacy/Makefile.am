
SUBDIRS = libormarketmodels

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp

libLegacy_la_SOURCES =

libLegacy_la_LIBADD = \
    libormarketmodels/libLiborMarketModels.la

noinst_LTLIBRARIES = libLegacy.la

include ${top_srcdir}/mk/all.mk

