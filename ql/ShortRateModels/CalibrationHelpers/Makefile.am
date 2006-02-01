
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels/CalibrationHelpers

include_HEADERS = \
    all.hpp \
    caphelper.hpp \
    hestonmodelhelper.hpp \
    swaptionhelper.hpp

libCalibrationHelpers_la_SOURCES = \
    caphelper.cpp \
    hestonmodelhelper.cpp \
    swaptionhelper.cpp

noinst_LTLIBRARIES = libCalibrationHelpers.la

EXTRA_DIST = makefile.mak

