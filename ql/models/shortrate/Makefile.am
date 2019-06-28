
SUBDIRS = calibrationhelpers onefactormodels twofactormodels

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    onefactormodel.hpp \
    twofactormodel.hpp

cpp_files = \
    onefactormodel.cpp \
    twofactormodel.cpp

if UNITY_BUILD

nodist_libShortRateModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libShortRateModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libShortRateModels.la

libShortRateModels_la_LIBADD = \
    calibrationhelpers/libCalibrationHelpers.la \
    onefactormodels/libOneFactorModels.la \
    twofactormodels/libTwoFactorModels.la

include ${top_srcdir}/mk/all.mk

