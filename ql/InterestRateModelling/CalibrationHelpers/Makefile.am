# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling/CalibrationHelpers

include_HEADERS = \
    cap.hpp \
    swaption.hpp

EXTRA_DIST=makefile.mak

libCalibrationHelpers_la_SOURCES = \
    cap.cpp \
    swaption.cpp

noinst_LTLIBRARIES = libCalibrationHelpers.la
