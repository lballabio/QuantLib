
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    compositequote.hpp \
    derivedquote.hpp \
    eurodollarfuturesquote.hpp \
    forwardswapquote.hpp \
    forwardvaluequote.hpp \
    futuresconvadjustmentquote.hpp \
    impliedstddevquote.hpp \
    lastfixingquote.hpp \
    simplequote.hpp

cpp_files = \
    eurodollarfuturesquote.cpp \
    forwardswapquote.cpp \
    forwardvaluequote.cpp \
    futuresconvadjustmentquote.cpp \
    impliedstddevquote.cpp \
    lastfixingquote.cpp

if UNITY_BUILD

nodist_libQuotes_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libQuotes_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libQuotes.la

include ${top_srcdir}/mk/all.mk

