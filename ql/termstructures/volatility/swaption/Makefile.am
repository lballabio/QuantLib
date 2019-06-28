
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    cmsmarket.hpp \
    cmsmarketcalibration.hpp \
    gaussian1dswaptionvolatility.hpp \
    spreadedswaptionvol.hpp \
    swaptionconstantvol.hpp \
    swaptionvolcube.hpp \
    swaptionvolcube1.hpp \
    swaptionvolcube2.hpp \
    swaptionvoldiscrete.hpp \
    swaptionvolmatrix.hpp \
    swaptionvolstructure.hpp

cpp_files = \
    cmsmarket.cpp \
    cmsmarketcalibration.cpp \
    gaussian1dswaptionvolatility.cpp \
    spreadedswaptionvol.cpp \
    swaptionconstantvol.cpp \
    swaptionvolcube.cpp \
    swaptionvolcube2.cpp \
    swaptionvoldiscrete.cpp \
    swaptionvolmatrix.cpp \
    swaptionvolstructure.cpp

if UNITY_BUILD

nodist_libSwaptionVol_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libSwaptionVol_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libSwaptionVol.la

include ${top_srcdir}/mk/all.mk

