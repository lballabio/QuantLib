
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    onestepcoinitialswaps.hpp \
    onestepcoterminalswaps.hpp \
    onestepforwards.hpp \
    onestepoptionlets.hpp

cpp_files = \
    onestepcoinitialswaps.cpp \
    onestepcoterminalswaps.cpp \
    onestepforwards.cpp \
    onestepoptionlets.cpp

if UNITY_BUILD

nodist_libMarketModelsProductsOneStep_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsProductsOneStep_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsProductsOneStep.la

include ${top_srcdir}/mk/all.mk

