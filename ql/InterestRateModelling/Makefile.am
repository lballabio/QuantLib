# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling

SUBDIRS = CalibrationHelpers OneFactorModels TwoFactorModels

include_HEADERS = \
    calibrationhelper.hpp \
    grid.hpp \
    model.hpp \
    onefactormodel.hpp \
    twofactormodel.hpp \
    shortrateprocess.hpp \
    timefunction.hpp \
    trinomialtree.hpp

EXTRA_DIST=makefile.mak

libInterestRateModelling_la_SOURCES = \
    calibrationhelper.cpp \
    grid.cpp \
    model.cpp \
    timefunction.cpp \
    trinomialtree.cpp

noinst_LTLIBRARIES = libInterestRateModelling.la
