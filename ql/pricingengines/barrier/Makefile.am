
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
    analyticbarrierengine.hpp \
	analyticbinarybarrierengine.hpp \
    binomialbarrierengine.hpp \
    discretizedbarrieroption.hpp \
	fdblackscholesbarrierengine.hpp \
	fdblackscholesrebateengine.hpp \
	fdhestonbarrierengine.hpp \
	fdhestonrebateengine.hpp \
    mcbarrierengine.hpp

cpp_files = \
    analyticbarrierengine.cpp \
    analyticbinarybarrierengine.cpp \
    discretizedbarrieroption.cpp \
	fdblackscholesbarrierengine.cpp \
	fdblackscholesrebateengine.cpp \
	fdhestonbarrierengine.cpp \
	fdhestonrebateengine.cpp \
    mcbarrierengine.cpp

if UNITY_BUILD

nodist_libBarrierEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libBarrierEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libBarrierEngines.la

include ${top_srcdir}/mk/all.mk

