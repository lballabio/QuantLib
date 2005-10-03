
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels/OneFactorModels

include_HEADERS = \
    blackkarasinski.hpp \
    coxingersollross.hpp \
    extendedcoxingersollross.hpp \
    hullwhite.hpp \
    vasicek.hpp

libOneFactorModels_la_SOURCES = \
    blackkarasinski.cpp \
    coxingersollross.cpp \
    extendedcoxingersollross.cpp \
    hullwhite.cpp \
    vasicek.cpp

noinst_LTLIBRARIES = libOneFactorModels.la

EXTRA_DIST = makefile.mak

