
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products

include_HEADERS = \
    all.hpp \
    marketmodelcaplets.hpp \
    marketmodelcapletsonestep.hpp \
    marketmodelcoinitialswaps.hpp \
    marketmodelcoterminalswaps.hpp \
    marketmodelforwards.hpp \
    marketmodelforwardsonestep.hpp \
    marketmodelratchet.hpp

libMarketModelsProducts_la_SOURCES = \
    marketmodelcaplets.cpp \
    marketmodelcapletsonestep.cpp \
    marketmodelcoinitialswaps.cpp \
    marketmodelcoterminalswaps.cpp \
    marketmodelforwards.cpp \
    marketmodelforwardsonestep.cpp \
    marketmodelratchet.cpp

noinst_LTLIBRARIES = libMarketModelsProducts.la

EXTRA_DIST = makefile.mak

