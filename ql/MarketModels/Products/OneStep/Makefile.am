
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products/OneStep

include_HEADERS = \
    all.hpp \
    onestepcaplets.hpp \
    onestepcoinitialswaps.hpp \
    onestepcoterminalswaps.hpp \
    onestepforwards.hpp

libMarketModelsProductsOneStep_la_SOURCES = \
    onestepcaplets.cpp \
    onestepcoinitialswaps.cpp \
    onestepcoterminalswaps.cpp \
    onestepforwards.cpp

noinst_LTLIBRARIES = libMarketModelsProductsOneStep.la

#EXTRA_DIST = makefile.mak
