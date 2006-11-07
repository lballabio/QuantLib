
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Models

include_HEADERS = \
	all.hpp \
	abcd.hpp \
	expcorrabcdvol.hpp \
	expcorrflatvol.hpp

libMarketModelsModels_la_SOURCES = \
	abcd.cpp \
	expcorrabcdvol.cpp \
	expcorrflatvol.cpp

noinst_LTLIBRARIES = libMarketModelsModels.la

EXTRA_DIST = makefile.mak

