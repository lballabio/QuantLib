
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    callspecifiedmultiproduct.hpp \
    cashrebate.hpp \
    exerciseadapter.hpp \
    multistepcoinitialswaps.hpp \
    multistepcoterminalswaps.hpp \
    multistepcoterminalswaptions.hpp \
    multistepforwards.hpp \
    multistepinversefloater.hpp \
    multistepnothing.hpp \
    multistepoptionlets.hpp \
    multisteppathwisewrapper.hpp \
    multistepperiodcapletswaptions.hpp \
    multistepratchet.hpp \
    multistepswap.hpp \
    multistepswaption.hpp \
    multisteptarn.hpp

cpp_files = \
    callspecifiedmultiproduct.cpp \
    cashrebate.cpp \
    exerciseadapter.cpp \
    multistepcoinitialswaps.cpp \
    multistepcoterminalswaps.cpp \
    multistepcoterminalswaptions.cpp \
    multistepforwards.cpp \
    multistepinversefloater.cpp \
    multistepnothing.cpp \
    multistepoptionlets.cpp \
    multisteppathwisewrapper.cpp \
    multistepperiodcapletswaptions.cpp \
    multistepratchet.cpp \
    multistepswap.cpp \
    multistepswaption.cpp \
    multisteptarn.cpp

if UNITY_BUILD

nodist_libMarketModelsProductsMultiStep_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsProductsMultiStep_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsProductsMultiStep.la

include ${top_srcdir}/mk/all.mk

