
SUBDIRS = onestep multistep

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    compositeproduct.hpp \
    multiproductcomposite.hpp \
    multiproductmultistep.hpp \
    multiproductonestep.hpp \
    singleproductcomposite.hpp

libMarketModelsProducts_la_SOURCES = \
    compositeproduct.cpp \
    multiproductcomposite.cpp \
    multiproductmultistep.cpp \
    multiproductonestep.cpp \
    singleproductcomposite.cpp

noinst_LTLIBRARIES = libMarketModelsProducts.la

libMarketModelsProducts_la_LIBADD = \
    multistep/libMarketModelsProductsMultiStep.la \
    onestep/libMarketModelsProductsOneStep.la

