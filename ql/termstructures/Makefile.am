
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS = volatilities yieldcurves

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp

libTermStructures_la_SOURCES =

libTermStructures_la_LIBADD = \
    volatilities/libVolatilities.la \
    yieldcurves/libYieldCurves.la

noinst_LTLIBRARIES = libTermStructures.la

