
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels/TwoFactorModels

include_HEADERS = \
    g2.hpp \
    hestonmodel.hpp

EXTRA_DIST = makefile.mak

libTwoFactorModels_la_SOURCES = \
    g2.cpp \
    hestonmodel.cpp

noinst_LTLIBRARIES = libTwoFactorModels.la
