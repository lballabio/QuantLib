
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    blackkarasinski.hpp \
    coxingersollross.hpp \
    extendedcoxingersollross.hpp \
    gaussian1dmodel.hpp \
    gsr.hpp \
    hullwhite.hpp \
    markovfunctional.hpp \
    vasicek.hpp

cpp_files = \
    blackkarasinski.cpp \
    coxingersollross.cpp \
    extendedcoxingersollross.cpp \
    gaussian1dmodel.cpp \
    gsr.cpp \
    hullwhite.cpp \
    markovfunctional.cpp \
    vasicek.cpp

if UNITY_BUILD

nodist_libOneFactorModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libOneFactorModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libOneFactorModels.la

include ${top_srcdir}/mk/all.mk

