# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels

SUBDIRS = CalibrationHelpers OneFactorModels TwoFactorModels

include_HEADERS = \
    calibrationhelper.hpp \
    model.hpp \
    onefactormodel.hpp \
    parameter.hpp \
    twofactormodel.hpp

EXTRA_DIST=makefile.mak

libShortRateModels_la_SOURCES = \
    calibrationhelper.cpp \
    model.cpp \
    onefactormodel.cpp \
    twofactormodel.cpp

noinst_LTLIBRARIES = libShortRateModels.la
