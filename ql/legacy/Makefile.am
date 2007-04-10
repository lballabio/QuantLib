
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/legacy

SUBDIRS = libormarketmodels pricers

include_HEADERS = \
	all.hpp

libLegacy_la_SOURCES =

libLegacy_la_LIBADD = \
    libormarketmodels/libLiborMarketModels.la \
    pricers/libPricers.la

noinst_LTLIBRARIES = libLegacy.la

