
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Barrier

include_HEADERS = \
	all.hpp \
    analyticamericanbinarybarrierengine.hpp \
    analyticbarrierengine.hpp \
    analyticeuropeanbinarybarrierengine.hpp \
    mcbarrierengine.hpp \
    mcbinarybarrierengine.hpp

EXTRA_DIST=makefile.mak

libBarrierEngines_la_SOURCES = \
    analyticamericanbinarybarrierengine.cpp \
    analyticbarrierengine.cpp \
    analyticeuropeanbinarybarrierengine.cpp

noinst_LTLIBRARIES = libBarrierEngines.la
