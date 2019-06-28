
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    swaptioncfs.hpp \
    tenoroptionletvts.hpp \
    tenorswaptionvts.hpp

cpp_files = \
    swaptioncfs.cpp \
    tenoroptionletvts.cpp \
    tenorswaptionvts.cpp

if UNITY_BUILD

nodist_libBasisModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libBasisModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libBasisModels.la

include ${top_srcdir}/mk/all.mk

