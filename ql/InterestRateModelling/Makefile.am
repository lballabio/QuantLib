# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling

SUBDIRS = CalibrationHelpers OneFactorModels

include_HEADERS = \
    grid.hpp \
    model.hpp \
    onefactormodel.hpp \
    shortrateprocess.hpp \
    timefunction.hpp \
    trinomialtree.hpp

EXTRA_DIST=makefile.mak

libInterestRateModelling_la_SOURCES = \
    model.cpp \
    onefactormodel.cpp \
    trinomialtree.cpp

noinst_LTLIBRARIES = libInterestRateModelling.la
