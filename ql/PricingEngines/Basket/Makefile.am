
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Basket

include_HEADERS = \
	all.hpp \
	mcamericanbasketengine.hpp \
	mcbasketengine.hpp \
	stulzengine.hpp

EXTRA_DIST=makefile.mak

libBasketEngines_la_SOURCES = \
	mcamericanbasketengine.cpp \
	stulzengine.cpp

noinst_LTLIBRARIES = libBasketEngines.la
