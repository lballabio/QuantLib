
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} -I${top_srcdir}/functions
includedir = $(prefix)/include/ql/Functions

include_HEADERS = \
	all.hpp \
	daycounters.hpp \
	mathf.hpp \
	vols.hpp

EXTRA_DIST=makefile.mak

lib_LTLIBRARIES = libQuantLibFunctions.la
libQuantLibFunctions_la_LDFLAGS = -release $(PACKAGE_VERSION)

libQuantLibFunctions_la_SOURCES = \
    daycounters.cpp \
    mathf.cpp \
	vols.cpp

