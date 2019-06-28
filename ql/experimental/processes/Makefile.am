
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    extouwithjumpsprocess.hpp \
    extendedblackscholesprocess.hpp \
    extendedornsteinuhlenbeckprocess.hpp \
    gemanroncoroniprocess.hpp \
    hestonslvprocess.hpp \
    klugeextouprocess.hpp \
    vegastressedblackscholesprocess.hpp

cpp_files = \
	extouwithjumpsprocess.cpp \
    extendedblackscholesprocess.cpp \
    extendedornsteinuhlenbeckprocess.cpp \
    gemanroncoroniprocess.cpp \
    hestonslvprocess.cpp \
    klugeextouprocess.cpp \
    vegastressedblackscholesprocess.cpp

if UNITY_BUILD

nodist_libProcesses_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libProcesses_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libProcesses.la

include ${top_srcdir}/mk/all.mk

