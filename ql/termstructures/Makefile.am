
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/termstructures

SUBDIRS = volatilities yieldcurves

include_HEADERS = \
    all.hpp

libTermStructures_la_SOURCES =

libTermStructures_la_LIBADD = \
    volatilities/libVolatilities.la \
    yieldcurves/libYieldCurves.la

noinst_LTLIBRARIES = libTermStructures.la

