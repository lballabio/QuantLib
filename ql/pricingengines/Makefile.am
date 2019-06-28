
SUBDIRS = asian barrier basket bond capfloor cliquet credit \
          forward inflation lookback quanto swap swaption vanilla

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    americanpayoffatexpiry.hpp \
    americanpayoffathit.hpp \
    blackcalculator.hpp \
    blackformula.hpp \
    blackscholescalculator.hpp \
    genericmodelengine.hpp \
    greeks.hpp \
    latticeshortratemodelengine.hpp \
    mclongstaffschwartzengine.hpp \
    mcsimulation.hpp

cpp_files = \
	americanpayoffatexpiry.cpp \
	americanpayoffathit.cpp \
	blackcalculator.cpp \
	blackformula.cpp \
	blackscholescalculator.cpp \
	greeks.cpp

if UNITY_BUILD

nodist_libPricingEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libPricingEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libPricingEngines.la

libPricingEngines_la_LIBADD = \
    asian/libAsianEngines.la \
    barrier/libBarrierEngines.la \
    basket/libBasketEngines.la \
    bond/libBondEngines.la \
    capfloor/libCapFloorEngines.la \
    cliquet/libCliquetEngines.la \
    credit/libCreditEngines.la \
    inflation/libInflationEngines.la \
    lookback/libLookbackEngines.la \
    swap/libSwapEngines.la \
    swaption/libSwaptionEngines.la \
    vanilla/libVanillaEngines.la

include ${top_srcdir}/mk/all.mk

