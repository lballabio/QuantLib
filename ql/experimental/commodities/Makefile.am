
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    commodity.hpp \
    commoditycashflow.hpp \
    commoditycurve.hpp \
    commodityindex.hpp \
    commoditypricinghelpers.hpp \
    commoditysettings.hpp \
    commoditytype.hpp \
    commodityunitcost.hpp \
    dateinterval.hpp \
    energybasisswap.hpp \
    energycommodity.hpp \
    energyfuture.hpp \
    energyswap.hpp \
    energyvanillaswap.hpp \
    exchangecontract.hpp \
    paymentterm.hpp \
    petroleumunitsofmeasure.hpp \
    pricingperiod.hpp \
    quantity.hpp \
    unitofmeasure.hpp \
    unitofmeasureconversion.hpp \
    unitofmeasureconversionmanager.hpp

cpp_files = \
    commodity.cpp \
    commoditycashflow.cpp \
    commoditycurve.cpp \
    commodityindex.cpp \
    commoditypricinghelpers.cpp \
    commoditysettings.cpp \
    commoditytype.cpp \
    commodityunitcost.cpp \
    dateinterval.cpp \
    energybasisswap.cpp \
    energycommodity.cpp \
    energyfuture.cpp \
    energyswap.cpp \
    energyvanillaswap.cpp \
    paymentterm.cpp \
    quantity.cpp \
    unitofmeasure.cpp \
    unitofmeasureconversion.cpp \
    unitofmeasureconversionmanager.cpp

if UNITY_BUILD

nodist_libCommodities_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCommodities_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCommodities.la

include ${top_srcdir}/mk/all.mk

