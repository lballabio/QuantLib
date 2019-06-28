
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    analyticcapfloorengine.hpp \
    blackcapfloorengine.hpp \
    bacheliercapfloorengine.hpp \
    discretizedcapfloor.hpp \
    gaussian1dcapfloorengine.hpp \
    mchullwhiteengine.hpp \
    treecapfloorengine.hpp

cpp_files = \
    analyticcapfloorengine.cpp \
    blackcapfloorengine.cpp \
    bacheliercapfloorengine.cpp \
    discretizedcapfloor.cpp \
    gaussian1dcapfloorengine.cpp \
    mchullwhiteengine.cpp \
    treecapfloorengine.cpp

if UNITY_BUILD

nodist_libCapFloorEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCapFloorEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCapFloorEngines.la

include ${top_srcdir}/mk/all.mk

