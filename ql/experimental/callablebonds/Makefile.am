
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    blackcallablebondengine.hpp \
    callablebondconstantvol.hpp \
    callablebond.hpp \
    callablebondvolstructure.hpp \
    discretizedcallablefixedratebond.hpp \
    treecallablebondengine.hpp

cpp_files = \
    blackcallablebondengine.cpp \
    callablebondconstantvol.cpp \
    callablebond.cpp \
    callablebondvolstructure.cpp \
    discretizedcallablefixedratebond.cpp \
    treecallablebondengine.cpp

if UNITY_BUILD

nodist_libCallableBonds_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCallableBonds_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCallableBonds.la

include ${top_srcdir}/mk/all.mk

