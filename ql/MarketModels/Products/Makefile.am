
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products

SUBDIRS = OneStep MultiStep

include_HEADERS = \
    all.hpp \
    marketmodelratchet.hpp \
    multiproductcomposite.hpp \
    multiproductmultistep.hpp \
    multiproductonestep.hpp

libMarketModelsProducts_la_SOURCES = \
    marketmodelratchet.cpp \
    multiproductcomposite.cpp \
    multiproductmultistep.cpp \
    multiproductonestep.cpp

noinst_LTLIBRARIES = libMarketModelsProducts.la

libMarketModels_la_LIBADD = \
    MultiStep/libMarketModelsProductsMultiStep.la \
    OneStep/libMarketModelsProductsOneStep.la

#EXTRA_DIST = makefile.mak
