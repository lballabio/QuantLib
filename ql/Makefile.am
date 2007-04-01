
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql

SUBDIRS = \
    calendars \
    cashflows \
    currencies \
    daycounters \
    finitedifferences \
    indexes \
    instruments \
    lattices \
    marketmodels \
    math \
    montecarlo \
    optimization \
    patterns \
    pricers \
    pricingengines \
    processes \
    quotes \
    randomnumbers \
    shortratemodels \
    solvers1d \
    termstructures \
    utilities \
    volatilities \
    volatilitymodels

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
	businessdayconvention.hpp \
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
    calendars/libCalendars.la \
    cashflows/libCashFlows.la \
    currencies/libCurrencies.la \
    daycounters/libDayCounters.la \
    finitedifferences/libFiniteDifferences.la \
    indexes/libIndexes.la \
    instruments/libInstruments.la \
    lattices/libLattices.la \
    math/libMath.la \
    marketmodels/libMarketModels.la \
    montecarlo/libMonteCarlo.la \
    optimization/libOptimization.la \
    pricers/libPricers.la \
    pricingengines/libPricingEngines.la \
    processes/libProcesses.la \
    quotes/libQuotes.la \
    randomnumbers/libRandomNumbers.la \
    shortratemodels/libShortRateModels.la \
    termstructures/libTermStructures.la \
    utilities/libUtilities.la \
    volatilities/libVolatilities.la \
    volatilitymodels/libVolatilityModels.la

libQuantLib_la_SOURCES = \
	businessdayconvention.cpp \
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
