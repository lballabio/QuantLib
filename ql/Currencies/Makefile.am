
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Currencies

include_HEADERS = \
	all.hpp \
	africa.hpp \
	america.hpp \
	asia.hpp \
	europe.hpp \
	exchangeratemanager.hpp \
	oceania.hpp

EXTRA_DIST = makefile.mak

libCurrencies_la_SOURCES = \
	exchangeratemanager.cpp

noinst_LTLIBRARIES = libCurrencies.la
