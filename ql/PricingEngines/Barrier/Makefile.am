
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Barrier

include_HEADERS = \
	all.hpp \
    analyticbarrierengine.hpp \
    mcbarrierengine.hpp

libBarrierEngines_la_SOURCES = \
    analyticbarrierengine.cpp \
	mcbarrierengine.cpp

noinst_LTLIBRARIES = libBarrierEngines.la
