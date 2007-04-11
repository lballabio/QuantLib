
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	binomialdistribution.hpp \
	bivariatenormaldistribution.hpp \
	chisquaredistribution.hpp \
	gammadistribution.hpp \
	normaldistribution.hpp \
	poissondistribution.hpp

libDistributions_la_SOURCES = \
	bivariatenormaldistribution.cpp \
	chisquaredistribution.cpp \
	gammadistribution.cpp \
	normaldistribution.cpp 

noinst_LTLIBRARIES = libDistributions.la

