
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Basket

include_HEADERS = \
	all.hpp \
	mcbasketengine.hpp \
	stulzengine.hpp

EXTRA_DIST=makefile.mak

libBasketEngines_la_SOURCES = \
	stulzengine.cpp

noinst_LTLIBRARIES = libBasketEngines.la
