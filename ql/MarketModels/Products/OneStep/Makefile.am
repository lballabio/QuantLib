
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products/OneStep

include_HEADERS = \
    all.hpp \
    onestepcoinitialswaps.hpp \
    onestepcoterminalswaps.hpp \
    onestepforwards.hpp \
    onestepoptionlets.hpp

libMarketModelsProductsOneStep_la_SOURCES = \
    onestepcoinitialswaps.cpp \
    onestepcoterminalswaps.cpp \
    onestepforwards.cpp \
    onestepoptionlets.cpp

noinst_LTLIBRARIES = libMarketModelsProductsOneStep.la

EXTRA_DIST = makefile.mak

