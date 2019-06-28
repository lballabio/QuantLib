
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	binomialdistribution.hpp \
	bivariatenormaldistribution.hpp \
	bivariatestudenttdistribution.hpp \
	chisquaredistribution.hpp \
	gammadistribution.hpp \
	normaldistribution.hpp \
	poissondistribution.hpp \
	studenttdistribution.hpp

cpp_files = \
	bivariatenormaldistribution.cpp \
	bivariatestudenttdistribution.cpp \
	chisquaredistribution.cpp \
	gammadistribution.cpp \
	normaldistribution.cpp \
	studenttdistribution.cpp

if UNITY_BUILD

nodist_libDistributions_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libDistributions_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libDistributions.la

include ${top_srcdir}/mk/all.mk

