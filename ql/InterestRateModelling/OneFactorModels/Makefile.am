
# $Id$

INCLUDES = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling/OneFactorModels

include_HEADERS = \
    blackkarasinski.hpp \
    coxingersollrossplus.hpp \
    hullwhite.hpp

EXTRA_DIST = makefile.mak

libOneFactorModels_la_SOURCES = \
    blackkarasinski.cpp \
    coxingersollrossplus.cpp \
    hullwhite.cpp

noinst_LTLIBRARIES = libOneFactorModels.la
