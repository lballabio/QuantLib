
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/termstructures

SUBDIRS = volatilities yieldcurves

include_HEADERS = \
    all.hpp \
    bondhelpers.hpp \
    bootstraptraits.hpp \
    drifttermstructure.hpp \
    quantotermstructure.hpp \
    ratehelpers.hpp 

libTermStructures_la_SOURCES = \
    bondhelpers.cpp \
    ratehelpers.cpp

libTermStructures_la_LIBADD = \
    volatilities/libVolatilities.la \
    yieldcurves/libYieldCurves.la

noinst_LTLIBRARIES = libTermStructures.la

