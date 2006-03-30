
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/VolatilityModels

include_HEADERS = \
    constantestimator.hpp \
    garch.hpp

libVolatilityModels_la_SOURCES = \
    constantestimator.cpp

noinst_LTLIBRARIES = libVolatilityModels.la

EXTRA_DIST = makefile.mak

