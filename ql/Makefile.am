
SUBDIRS = \
    cashflows \
    currencies \
    indexes \
    instruments \
    legacy \
    marketmodels \
    math \
    methods \
    patterns \
    pricingengines \
    processes \
    quotes \
    shortratemodels \
    termstructures \
    time \
    utilities \
    volatilitymodels

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	core.hpp \
	auto_link.hpp \
	capvolstructures.hpp \
	cashflow.hpp \
	config.hpp \
	currency.hpp \
	daycounter.hpp \
	discretizedasset.hpp \
	errors.hpp \
	exchangerate.hpp \
	exercise.hpp \
	event.hpp \
	grid.hpp \
	handle.hpp \
	index.hpp \
	instrument.hpp \
	interestrate.hpp \
	money.hpp \
	numericalmethod.hpp \
	option.hpp \
	payoff.hpp \
	position.hpp \
	prices.hpp \
	pricingengine.hpp \
	qldefines.hpp \
	quantlib.hpp \
	quote.hpp \
	settings.hpp \
	solver1d.hpp \
	stochasticprocess.hpp \
	swaptionvolstructure.hpp \
	termstructure.hpp \
	timegrid.hpp \
	timeseries.hpp \
	types.hpp \
	volatilitymodel.hpp \
	voltermstructure.hpp \
	yieldtermstructure.hpp

libQuantLib_la_SOURCES = \
    currency.cpp \
	discretizedasset.cpp \
	errors.cpp \
	exchangerate.cpp \
	exercise.cpp \
    index.cpp \
    interestrate.cpp \
    money.cpp \
    prices.cpp \
	stochasticprocess.cpp \
	swaptionvolstructure.cpp \
	timegrid.cpp \
    voltermstructure.cpp

lib_LTLIBRARIES = libQuantLib.la
libQuantLib_la_LDFLAGS = -release $(PACKAGE_VERSION)

libQuantLib_la_LIBADD = \
    cashflows/libCashFlows.la \
    currencies/libCurrencies.la \
    indexes/libIndexes.la \
    instruments/libInstruments.la \
    legacy/libLegacy.la \
    math/libMath.la \
    marketmodels/libMarketModels.la \
    methods/libMethods.la \
    pricingengines/libPricingEngines.la \
    processes/libProcesses.la \
    quotes/libQuotes.la \
    shortratemodels/libShortRateModels.la \
    termstructures/libTermStructures.la \
    time/libTime.la \
    utilities/libUtilities.la \
    volatilitymodels/libVolatilityModels.la

EXTRA_DIST= \
	config.ansi.hpp \
	config.bcc.hpp \
	config.msvc.hpp \
	config.mwcw.hpp \
	config.mingw.hpp \
	userconfig.hpp

install-data-hook:
	$(SED) -e "s,HAVE_CONFIG_H,QL_HAVE_CONFIG_H," \
           -e "s,/\* install-hook \*/,#define QL_HAVE_CONFIG_H," \
           $(DESTDIR)/$(this_includedir)/qldefines.hpp > .qldefines.hpp
	$(INSTALL_DATA) .qldefines.hpp $(DESTDIR)/$(this_includedir)/qldefines.hpp
	rm .qldefines.hpp
	$(SED) -e "s,PACKAGE,QL_PACKAGE," \
           -e "s,STDC,QL_STDC," \
           -e "s, HAVE, QL_HAVE," \
           -e "s, VERSION, QL_AC_VERSION," \
           $(DESTDIR)/$(this_includedir)/config.hpp > .config.hpp
	$(INSTALL_DATA) .config.hpp $(DESTDIR)/$(this_includedir)/config.hpp
	rm .config.hpp
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)

if ENABLE_STATIC

install-exec-hook:
	mv $(DESTDIR)/$(libdir)/libQuantLib.a $(DESTDIR)/$(libdir)/libQuantLib-$(PACKAGE_VERSION).a
	$(LN_S) libQuantLib-$(PACKAGE_VERSION).a $(DESTDIR)/$(libdir)/libQuantLib.a
endif


