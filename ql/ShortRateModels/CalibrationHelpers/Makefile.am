# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels/CalibrationHelpers

include_HEADERS = \
    caphelper.hpp \
    swaptionhelper.hpp

EXTRA_DIST=makefile.mak

libCalibrationHelpers_la_SOURCES = \
    caphelper.cpp \
    swaptionhelper.cpp

noinst_LTLIBRARIES = libCalibrationHelpers.la
