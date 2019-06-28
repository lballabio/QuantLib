
SUBDIRS = credit inflation volatility yield

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	bootstraperror.hpp \
	bootstraphelper.hpp \
	defaulttermstructure.hpp \
	inflationtermstructure.hpp \
	interpolatedcurve.hpp \
	iterativebootstrap.hpp \
	localbootstrap.hpp \
	voltermstructure.hpp \
	yieldtermstructure.hpp

cpp_files = \
	defaulttermstructure.cpp \
	inflationtermstructure.cpp \
	voltermstructure.cpp \
	yieldtermstructure.cpp

if UNITY_BUILD

nodist_libTermStructures_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libTermStructures_la_SOURCES = $(cpp_files)

endif

libTermStructures_la_LIBADD = \
    credit/libDefaultTermStructures.la \
    inflation/libInflationTermStructures.la \
    yield/libYieldTermStructures.la \
    volatility/libVolTermStructures.la

noinst_LTLIBRARIES = libTermStructures.la

include ${top_srcdir}/mk/all.mk

