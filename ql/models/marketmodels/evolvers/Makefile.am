SUBDIRS = volprocesses

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	lognormalcmswapratepc.hpp \
	lognormalcotswapratepc.hpp \
	lognormalfwdrateballand.hpp \
	lognormalfwdrateeuler.hpp \
	lognormalfwdrateeulerconstrained.hpp \
	lognormalfwdrateiballand.hpp \
	lognormalfwdrateipc.hpp \
	lognormalfwdratepc.hpp \
	marketmodelvolprocess.hpp \
	normalfwdratepc.hpp \
	svddfwdratepc.hpp

cpp_files = \
	lognormalcmswapratepc.cpp \
	lognormalcotswapratepc.cpp \
	lognormalfwdrateballand.cpp \
	lognormalfwdrateeuler.cpp \
	lognormalfwdrateeulerconstrained.cpp \
	lognormalfwdrateiballand.cpp \
	lognormalfwdrateipc.cpp \
	lognormalfwdratepc.cpp \
	marketmodelvolprocess.cpp \
	normalfwdratepc.cpp \
	svddfwdratepc.cpp

if UNITY_BUILD

nodist_libMarketModelsEvolvers_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsEvolvers_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsEvolvers.la

libMarketModelsEvolvers_la_LIBADD = \
    volprocesses/libMarketModelsVolProcesses.la

include ${top_srcdir}/mk/all.mk

