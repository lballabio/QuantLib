
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} -I${top_srcdir}/functions
includedir = $(prefix)/include/ql/Functions

include_HEADERS = \
	calendars.hpp \
	daycounters.hpp \
	mathf.hpp \
	qlfunctions.hpp \
	vols.hpp

EXTRA_DIST = \
	makefile.mak \
	QuantLibFunctions.dsp \
	QuantLibFunctions.vcproj \
	QuantLibFunctions_vc8.vcproj \
	QuantLibFunctions.dev

lib_LTLIBRARIES = libQuantLibFunctions.la
libQuantLibFunctions_la_LDFLAGS = -release $(PACKAGE_VERSION)

libQuantLibFunctions_la_SOURCES = \
	calendars.cpp \
	daycounters.cpp \
	mathf.cpp \
	vols.cpp

dist-hook:
	mkdir -p $(distdir)/build
