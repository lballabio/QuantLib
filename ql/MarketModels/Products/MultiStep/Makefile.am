
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Products/MultiStep

include_HEADERS = \
    all.hpp \
    callspecifiedmultiproduct.hpp \
    cashrebate.hpp \
    exerciseadapter.hpp \
    multistepcoinitialswaps.hpp \
    multistepcoterminalswaps.hpp \
    multistepcoterminalswaptions.hpp \
    multistepforwards.hpp \
    multistepnothing.hpp \
    multistepoptionlets.hpp \
    multistepratchet.hpp \
    multistepswap.hpp

libMarketModelsProductsMultiStep_la_SOURCES = \
    callspecifiedmultiproduct.cpp \
    cashrebate.cpp \
    exerciseadapter.cpp \
    multistepcoinitialswaps.cpp \
    multistepcoterminalswaps.cpp \
    multistepcoterminalswaptions.cpp \
    multistepforwards.cpp \
    multistepnothing.cpp \
    multistepoptionlets.cpp \
    multistepratchet.cpp \
    multistepswap.cpp

noinst_LTLIBRARIES = libMarketModelsProductsMultiStep.la

EXTRA_DIST = makefile.mak

