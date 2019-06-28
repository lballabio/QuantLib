
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    composite.hpp \
    curiouslyrecurring.hpp \
    lazyobject.hpp \
    observable.hpp \
    singleton.hpp \
    visitor.hpp

cpp_files = \
	observable.cpp

if UNITY_BUILD

nodist_libPatterns_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libPatterns_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libPatterns.la

include ${top_srcdir}/mk/all.mk

