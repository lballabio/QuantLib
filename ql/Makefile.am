
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql

SUBDIRS = \
    Calendars \
    CashFlows \
    DayCounters \
    FiniteDifferences \
    functions \
    Indexes \
    Instruments \
    InterestRateModelling \
    Lattices \
    Math \
    MonteCarlo \
    Optimization \
    Patterns \
    Pricers \
    RandomNumbers \
    Solvers1D \
    TermStructures \
    Utilities \
    Volatilities

EXTRA_DIST= \
	config.ansi.hpp \
	config.bcc.hpp \
	config.decc.hpp \
	config.msvc.hpp \
	config.mwcw.hpp \
	makefile.mak

include_HEADERS = \
	argsandresults.hpp \
	array.hpp \
	calendar.hpp \
	capvolstructures.hpp \
	cashflow.hpp \
	config.hpp \
	currency.hpp \
	dataformatters.hpp \
	date.hpp \
	daycounter.hpp \
    diffusionprocess.hpp \
	errors.hpp \
    exercise.hpp \
	expressiontemplates.hpp \
    grid.hpp \
	handle.hpp \
	history.hpp \
	index.hpp \
	instrument.hpp \
	marketelement.hpp \
	null.hpp \
    numericalmethod.hpp \
	option.hpp \
	qldefines.hpp \
	quantlib.hpp \
	relinkablehandle.hpp \
	riskstatistics.hpp \
	scheduler.hpp \
	solver1d.hpp \
	swaptionvolstructure.hpp \
	termstructure.hpp \
	types.hpp


install-data-hook:
	$(SED) -e 's/HAVE_CONFIG_H/QL_HAVE_CONFIG_H/;/#ifndef quantlib_defines_h/{G;s/$$/#define QL_HAVE_CONFIG_H/;}' $(includedir)/qldefines.hpp > .qldefines.hpp
	$(INSTALL_DATA) .qldefines.hpp $(includedir)/qldefines.hpp
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)


libQuantLib_la_LIBADD = \
	Calendars/libCalendars.la \
	CashFlows/libCashFlows.la \
	DayCounters/libDayCounters.la \
	FiniteDifferences/libFiniteDifferences.la \
    functions/libfunctions.la \
	Indexes/libIndexes.la \
	Instruments/libInstruments.la \
    InterestRateModelling/libInterestRateModelling.la \
    InterestRateModelling/CalibrationHelpers/libCalibrationHelpers.la \
    InterestRateModelling/OneFactorModels/libOneFactorModels.la \
    InterestRateModelling/TwoFactorModels/libTwoFactorModels.la \
    Lattices/libLattices.la \
	Math/libMath.la \
	MonteCarlo/libMonteCarlo.la \
    Optimization/libOptimization.la \
	Pricers/libPricers.la \
	RandomNumbers/libRandomNumbers.la \
	Solvers1D/libSolvers1D.la \
	TermStructures/libTermStructures.la

libQuantLib_la_SOURCES = \
    calendar.cpp \
    dataformatters.cpp \
    date.cpp \
    option.cpp \
    scheduler.cpp \
    solver1d.cpp

lib_LTLIBRARIES = libQuantLib.la
