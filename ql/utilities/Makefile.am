
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    clone.hpp \
    dataformatters.hpp \
    dataparsers.hpp \
    disposable.hpp \
    null.hpp \
	null_deleter.hpp \
    observablevalue.hpp \
    steppingiterator.hpp \
    tracing.hpp \
    vectors.hpp

cpp_files = \
    dataformatters.cpp \
    dataparsers.cpp \
    tracing.cpp

if UNITY_BUILD

nodist_libUtilities_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libUtilities_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libUtilities.la

include ${top_srcdir}/mk/all.mk

