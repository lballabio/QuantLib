
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products/MultiStep

include_HEADERS = \
    all.hpp \
    multistepcaplets.hpp \
    multistepcoinitialswaps.hpp \
    multistepcoterminalswaps.hpp \
    multistepforwards.hpp

libMarketModelsProductsMultiStep_la_SOURCES = \
    multistepcaplets.cpp \
    multistepcoinitialswaps.cpp \
    multistepcoterminalswaps.cpp \
    multistepforwards.cpp

noinst_LTLIBRARIES = libMarketModelsProductsMultiStep.la

#EXTRA_DIST = makefile.mak
