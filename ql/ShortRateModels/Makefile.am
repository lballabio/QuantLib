
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels

SUBDIRS = CalibrationHelpers OneFactorModels TwoFactorModels

include_HEADERS = \
    all.hpp \
    core.hpp \
    calibrationhelper.hpp \
    model.hpp \
    onefactormodel.hpp \
    parameter.hpp \
    twofactormodel.hpp

libShortRateModels_la_SOURCES = \
    calibrationhelper.cpp \
    model.cpp \
    onefactormodel.cpp \
    twofactormodel.cpp

noinst_LTLIBRARIES = libShortRateModels.la

libShortRateModels_la_LIBADD = \
    CalibrationHelpers/libCalibrationHelpers.la \
    OneFactorModels/libOneFactorModels.la \
    TwoFactorModels/libTwoFactorModels.la

EXTRA_DIST = makefile.mak

