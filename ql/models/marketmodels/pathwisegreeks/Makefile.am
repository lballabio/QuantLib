AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    bumpinstrumentjacobian.hpp \
    ratepseudorootjacobian.hpp \
    swaptionpseudojacobian.hpp \
    vegabumpcluster.hpp

cpp_files = \
    bumpinstrumentjacobian.cpp \
    ratepseudorootjacobian.cpp \
    swaptionpseudojacobian.cpp \
    vegabumpcluster.cpp

if UNITY_BUILD

nodist_libMarketModelsPathwiseGreeks_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsPathwiseGreeks_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsPathwiseGreeks.la

include ${top_srcdir}/mk/all.mk

