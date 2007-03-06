
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql

SUBDIRS = \
    Calendars \
    CashFlows \
    Currencies \
    DayCounters \
    FiniteDifferences \
    Indexes \
    Instruments \
    Lattices \
    MarketModels \
    Math \
    MonteCarlo \
    Optimization \
    Patterns \
    Pricers \
    PricingEngines \
    Processes \
    Quotes \
    RandomNumbers \
    ShortRateModels \
    Solvers1D \
    TermStructures \
    Utilities \
    Volatilities \
    VolatilityModels

EXTRA_DIST= \
	config.ansi.hpp \
	config.bcc.hpp \
	config.msvc.hpp \
	config.mwcw.hpp \
	config.mingw.hpp \
	userconfig.hpp

include_HEADERS = \
	core.hpp \
	auto_link.hpp \
	calendar.hpp \
	capvolstructures.hpp \
	cashflow.hpp \
	config.hpp \
	currency.hpp \
	date.hpp \
	daycounter.hpp \
	discretizedasset.hpp \
	errors.hpp \
	exchangerate.hpp \
	exercise.hpp \
	event.hpp \
	frequency.hpp \
	grid.hpp \
	handle.hpp \
	index.hpp \
	instrument.hpp \
	interestrate.hpp \
	money.hpp \
	numericalmethod.hpp \
	option.hpp \
	payoff.hpp \
	period.hpp \
	position.hpp \
	prices.hpp \
	pricingengine.hpp \
	qldefines.hpp \
	quantlib.hpp \
	quote.hpp \
	schedule.hpp \
	settings.hpp \
	solver1d.hpp \
	stochasticprocess.hpp \
	swaptionvolstructure.hpp \
	termstructure.hpp \
	timegrid.hpp \
	timeseries.hpp \
	timeunit.hpp \
	types.hpp \
	volatilitymodel.hpp \
	voltermstructure.hpp \
	weekday.hpp \
	yieldtermstructure.hpp


install-data-hook:
	$(SED) -e "s,HAVE_CONFIG_H,QL_HAVE_CONFIG_H," \
           -e "s,/\* install-hook \*/,#define QL_HAVE_CONFIG_H," \
           $(DESTDIR)/$(includedir)/qldefines.hpp > .qldefines.hpp
	$(INSTALL_DATA) .qldefines.hpp $(DESTDIR)/$(includedir)/qldefines.hpp
	rm .qldefines.hpp
	$(SED) -e "s,PACKAGE,QL_PACKAGE," \
           -e "s,STDC,QL_STDC," \
           -e "s, HAVE, QL_HAVE," \
           -e "s, VERSION, QL_AC_VERSION," \
           $(DESTDIR)/$(includedir)/config.hpp > .config.hpp
	$(INSTALL_DATA) .config.hpp $(DESTDIR)/$(includedir)/config.hpp
	rm .config.hpp
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)

if ENABLE_STATIC

install-exec-hook:
	mv $(DESTDIR)/$(libdir)/libQuantLib.a $(DESTDIR)/$(libdir)/libQuantLib-$(PACKAGE_VERSION).a
	$(LN_S) libQuantLib-$(PACKAGE_VERSION).a $(DESTDIR)/$(libdir)/libQuantLib.a
endif


lib_LTLIBRARIES = libQuantLib.la
libQuantLib_la_LDFLAGS = -release $(PACKAGE_VERSION)

libQuantLib_la_LIBADD = \
    Calendars/libCalendars.la \
    CashFlows/libCashFlows.la \
    Currencies/libCurrencies.la \
    DayCounters/libDayCounters.la \
    FiniteDifferences/libFiniteDifferences.la \
    Indexes/libIndexes.la \
    Instruments/libInstruments.la \
    Lattices/libLattices.la \
    Math/libMath.la \
    MarketModels/libMarketModels.la \
    MonteCarlo/libMonteCarlo.la \
    Optimization/libOptimization.la \
    Pricers/libPricers.la \
    PricingEngines/libPricingEngines.la \
    Processes/libProcesses.la \
    Quotes/libQuotes.la \
    RandomNumbers/libRandomNumbers.la \
    ShortRateModels/libShortRateModels.la \
    TermStructures/libTermStructures.la \
    Utilities/libUtilities.la \
    Volatilities/libVolatilities.la \
    VolatilityModels/libVolatilityModels.la

libQuantLib_la_SOURCES = \
    calendar.cpp \
    currency.cpp \
    date.cpp \
	discretizedasset.cpp \
	errors.cpp \
	exchangerate.cpp \
	exercise.cpp \
	frequency.cpp \
    index.cpp \
    interestrate.cpp \
    money.cpp \
	period.cpp \
    prices.cpp \
    schedule.cpp \
	stochasticprocess.cpp \
	swaptionvolstructure.cpp \
	timegrid.cpp \
    voltermstructure.cpp \
    weekday.cpp
