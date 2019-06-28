
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    integralcdsengine.hpp \
    isdacdsengine.hpp \
    midpointcdsengine.hpp

cpp_files = \
    integralcdsengine.cpp \
    isdacdsengine.cpp \
    midpointcdsengine.cpp

if UNITY_BUILD

nodist_libCreditEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCreditEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCreditEngines.la

include ${top_srcdir}/mk/all.mk

