
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

SUBDIRS = Asian Barrier Basket CapFloor Forward Quanto Swaption Vanilla

include_HEADERS = \
    all.hpp \
    americanpayoffatexpiry.hpp \
    americanpayoffathit.hpp \
    blackformula.hpp \
    blackmodel.hpp \
    core.hpp \
    genericmodelengine.hpp \
    latticeshortratemodelengine.hpp \
    mcsimulation.hpp

EXTRA_DIST=makefile.mak

libPricingEngines_la_SOURCES = \
	blackformula.cpp

noinst_LTLIBRARIES = libPricingEngines.la

libPricingEngines_la_LIBADD = \
    Asian/libAsianEngines.la \
    Barrier/libBarrierEngines.la \
    Basket/libBasketEngines.la \
    CapFloor/libCapFloorEngines.la \
    Swaption/libSwaptionEngines.la \
    Vanilla/libVanillaEngines.la
