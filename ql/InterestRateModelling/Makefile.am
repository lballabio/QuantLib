# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling

SUBDIRS = CalibrationHelpers OneFactorModels TwoFactorModels

include_HEADERS = \
    calibrationhelper.hpp \
    model.hpp \
    onefactormodel.hpp \
    parameter.hpp \
    shortrateprocess.hpp \
    twofactormodel.hpp

EXTRA_DIST=makefile.mak

libInterestRateModelling_la_SOURCES = \
    calibrationhelper.cpp \
    model.cpp \
    onefactormodel.cpp

noinst_LTLIBRARIES = libInterestRateModelling.la
