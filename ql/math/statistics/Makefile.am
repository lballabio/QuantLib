
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	convergencestatistics.hpp \
	discrepancystatistics.hpp \
	gaussianstatistics.hpp \
	generalstatistics.hpp \
	histogram.hpp \
	incrementalstatistics.hpp \
	riskstatistics.hpp \
	sequencestatistics.hpp \
	statistics.hpp

cpp_files = \
    discrepancystatistics.cpp \
    generalstatistics.cpp \
    histogram.cpp \
	incrementalstatistics.cpp

if UNITY_BUILD

nodist_libStatistics_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libStatistics_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libStatistics.la

include ${top_srcdir}/mk/all.mk

