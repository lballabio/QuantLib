
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Basket

include_HEADERS = \
	all.hpp \
	mcamericanbasketengine.hpp \
	mcbasketengine.hpp \
	stulzengine.hpp

libBasketEngines_la_SOURCES = \
	mcamericanbasketengine.cpp \
	mcbasketengine.cpp \
	stulzengine.cpp

noinst_LTLIBRARIES = libBasketEngines.la
