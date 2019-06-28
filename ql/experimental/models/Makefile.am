
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    hestonslvfdmmodel.hpp \
    hestonslvmcmodel.hpp \
    normalclvmodel.hpp \
    squarerootclvmodel.hpp

cpp_files = \
    hestonslvfdmmodel.cpp \
    hestonslvmcmodel.cpp \
    normalclvmodel.cpp \
    squarerootclvmodel.cpp

if UNITY_BUILD

nodist_libModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libModels.la

include ${top_srcdir}/mk/all.mk

