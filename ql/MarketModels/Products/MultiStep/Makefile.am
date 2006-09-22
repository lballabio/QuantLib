
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products/MultiStep

include_HEADERS = \
    all.hpp \
    callspecifiedmultiproduct.hpp \
    cashrebate.hpp \
    exerciseadapter.hpp \
    multistepcaplets.hpp \
    multistepcoinitialswaps.hpp \
    multistepcoterminalswaps.hpp \
    multistepforwards.hpp \
    multistepnothing.hpp \
    multistepswap.hpp

libMarketModelsProductsMultiStep_la_SOURCES = \
    callspecifiedmultiproduct.cpp \
    cashrebate.cpp \
    exerciseadapter.cpp \
    multistepcaplets.cpp \
    multistepcoinitialswaps.cpp \
    multistepcoterminalswaps.cpp \
    multistepforwards.cpp \
    multistepnothing.cpp \
    multistepswap.cpp

noinst_LTLIBRARIES = libMarketModelsProductsMultiStep.la

#EXTRA_DIST = makefile.mak
