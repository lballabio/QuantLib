
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/pricingengines

SUBDIRS = asian barrier basket capfloor cliquet \
          forward hybrid lookback quanto swaption vanilla

include_HEADERS = \
    all.hpp \
    americanpayoffatexpiry.hpp \
    americanpayoffathit.hpp \
    blackcalculator.hpp \
    blackformula.hpp \
    blackscholescalculator.hpp \
    core.hpp \
    genericmodelengine.hpp \
    greeks.hpp \
    latticeshortratemodelengine.hpp \
    mclongstaffschwartzengine.hpp \
    mcsimulation.hpp

libPricingEngines_la_SOURCES = \
	americanpayoffatexpiry.cpp \
	americanpayoffathit.cpp \
	blackcalculator.cpp \
	blackformula.cpp \
	blackscholescalculator.cpp \
	greeks.cpp

noinst_LTLIBRARIES = libPricingEngines.la

libPricingEngines_la_LIBADD = \
    asian/libAsianEngines.la \
    barrier/libBarrierEngines.la \
    basket/libBasketEngines.la \
    capfloor/libCapFloorEngines.la \
    cliquet/libCliquetEngines.la \
    lookback/libLookbackEngines.la \
    swaption/libSwaptionEngines.la \
    vanilla/libVanillaEngines.la \
    hybrid/libHybridEngines.la

