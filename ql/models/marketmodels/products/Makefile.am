
SUBDIRS = onestep multistep pathwise

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    compositeproduct.hpp \
    multiproductcomposite.hpp \
    multiproductmultistep.hpp \
    multiproductonestep.hpp \
    singleproductcomposite.hpp

cpp_files = \
    compositeproduct.cpp \
    multiproductcomposite.cpp \
    multiproductmultistep.cpp \
    multiproductonestep.cpp \
    singleproductcomposite.cpp

if UNITY_BUILD

nodist_libMarketModelsProducts_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsProducts_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsProducts.la

libMarketModelsProducts_la_LIBADD = \
    multistep/libMarketModelsProductsMultiStep.la \
    onestep/libMarketModelsProductsOneStep.la \
    pathwise/libMarketModelsProductsPathwise.la

include ${top_srcdir}/mk/all.mk

