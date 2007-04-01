
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/shortratemodels

SUBDIRS = calibrationhelpers onefactormodels twofactormodels libormarketmodels

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
    calibrationhelpers/libCalibrationHelpers.la \
    onefactormodels/libOneFactorModels.la \
    twofactormodels/libTwoFactorModels.la \
    libormarketmodels/libLiborMarketModels.la

