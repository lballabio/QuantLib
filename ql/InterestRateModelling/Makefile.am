# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling

SUBDIRS = CalibrationHelpers OneFactorModels

include_HEADERS = \
    grid.hpp \
    model.hpp \
    onefactormodel.hpp \
    swapfuturevalue.hpp

EXTRA_DIST=makefile.mak

libInterestRateModelling_la_SOURCES = \
    model.cpp \
    onefactormodel.cpp \
    swapfuturevalue.cpp

noinst_LTLIBRARIES = libInterestRateModelling.la
