
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	fd2dblackscholesvanillaengine.hpp \
	kirkengine.hpp \
	mcamericanbasketengine.hpp \
	mceuropeanbasketengine.hpp \
	stulzengine.hpp

cpp_files = \
	fd2dblackscholesvanillaengine.cpp \
	kirkengine.cpp \
	mcamericanbasketengine.cpp \
	mceuropeanbasketengine.cpp \
	stulzengine.cpp

if UNITY_BUILD

nodist_libBasketEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libBasketEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libBasketEngines.la

include ${top_srcdir}/mk/all.mk

