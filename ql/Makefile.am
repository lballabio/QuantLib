
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql

SUBDIRS = \
    Calendars \
    CashFlows \
    DayCounters \
    FiniteDifferences \
    Indexes \
    Instruments \
    Lattices \
    Math \
    MonteCarlo \
    Optimization \
    Patterns \
    Pricers \
    PricingEngines \
    RandomNumbers \
    ShortRateModels \
    Solvers1D \
    TermStructures \
    Utilities \
    Volatilities

EXTRA_DIST= \
	config.ansi.hpp \
	config.bcc.hpp \
	config.msvc.hpp \
	config.mwcw.hpp \
	makefile.mak \
	userconfig.hpp

include_HEADERS = \
	core.hpp \
	argsandresults.hpp \
	basicdataformatters.hpp \
	calendar.hpp \
	capvolstructures.hpp \
	cashflow.hpp \
	config.hpp \
	currency.hpp \
	dataformatters.hpp \
	dataparsers.hpp \
	date.hpp \
	daycounter.hpp \
	discretizedasset.hpp \
	disposable.hpp \
	errors.hpp \
	exercise.hpp \
	grid.hpp \
	history.hpp \
	index.hpp \
	instrument.hpp \
	null.hpp \
	numericalmethod.hpp \
	option.hpp \
	payoff.hpp \
	pricingengine.hpp \
	qldefines.hpp \
	quantlib.hpp \
	quote.hpp \
	relinkablehandle.hpp \
	schedule.hpp \
	solver1d.hpp \
	stochasticprocess.hpp \
	swaptionvolstructure.hpp \
	termstructure.hpp \
	types.hpp \
	voltermstructure.hpp


install-data-hook:
	$(SED) -e "s,HAVE_CONFIG_H,QL_HAVE_CONFIG_H," \
               -e "s,/\* install-hook \*/,#define QL_HAVE_CONFIG_H," \
           $(includedir)/qldefines.hpp > .qldefines.hpp
	$(INSTALL_DATA) .qldefines.hpp $(includedir)/qldefines.hpp
	rm .qldefines.hpp
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)


lib_LTLIBRARIES = libQuantLib.la
libQuantLib_la_LDFLAGS = -release $(PACKAGE_VERSION)

libQuantLib_la_LIBADD = \
    Calendars/libCalendars.la \
    CashFlows/libCashFlows.la \
    DayCounters/libDayCounters.la \
    FiniteDifferences/libFiniteDifferences.la \
    Indexes/libIndexes.la \
    Instruments/libInstruments.la \
    Lattices/libLattices.la \
    Math/libMath.la \
    Optimization/libOptimization.la \
    Pricers/libPricers.la \
    PricingEngines/libPricingEngines.la \
    RandomNumbers/libRandomNumbers.la \
    ShortRateModels/libShortRateModels.la \
    TermStructures/libTermStructures.la \
    Volatilities/libVolatilities.la

libQuantLib_la_SOURCES = \
    basicdataformatters.cpp \
    calendar.cpp \
    dataformatters.cpp \
    dataparsers.cpp \
    date.cpp \
	discretizedasset.cpp \
	errors.cpp \
	exercise.cpp \
    grid.cpp \
    schedule.cpp \
	stochasticprocess.cpp \
    voltermstructure.cpp
