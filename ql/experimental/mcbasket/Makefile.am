
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    adaptedpathpayoff.hpp \
    longstaffschwartzmultipathpricer.hpp \
    mcamericanpathengine.hpp \
    mclongstaffschwartzpathengine.hpp \
    mcpathbasketengine.hpp \
    pathmultiassetoption.hpp \
    pathpayoff.hpp

cpp_files = \
    adaptedpathpayoff.cpp \
    longstaffschwartzmultipathpricer.cpp \
    mcpathbasketengine.cpp \
    pathmultiassetoption.cpp

if UNITY_BUILD

nodist_libMcBasket_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMcBasket_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMcBasket.la

include ${top_srcdir}/mk/all.mk

