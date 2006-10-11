
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products

SUBDIRS = OneStep MultiStep

include_HEADERS = \
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
    MultiStep/libMarketModelsProductsMultiStep.la \
    OneStep/libMarketModelsProductsOneStep.la

#EXTRA_DIST = makefile.mak
