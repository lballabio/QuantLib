
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql

SUBDIRS = \
    Calendars \
    CashFlows \
    DayCounters \
    FiniteDifferences \
    Indexes \
    Instruments \
    Math \
    MonteCarlo \
    Patterns \
    Pricers \
    Solvers1D \
    TermStructures \
    Utilities

EXTRA_DIST= \
	config.ansi.hpp \
	config.bcc.hpp \
	config.decc.hpp \
	config.msvc.hpp \
	config.mwcw.hpp \
	makefile.mak

include_HEADERS = \
	array.hpp \
	calendar.hpp \
	cashflow.hpp \
	config.hpp \
	currency.hpp \
	dataformatters.hpp \
	date.hpp \
	daycounter.hpp \
	depositrate.hpp \
	errors.hpp \
	expressiontemplates.hpp \
	forwardvolsurface.hpp \
	handle.hpp \
	history.hpp \
	index.hpp \
	instrument.hpp \
	marketelement.hpp \
	null.hpp \
	options.hpp \
	qldefines.hpp \
	quantlib.hpp \
	relinkablehandle.hpp \
	riskstatistics.hpp \
	scheduler.hpp \
	solver1d.hpp \
	swaptionvolsurface.hpp \
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
	Indexes/libIndexes.la \
	Instruments/libInstruments.la \
	Math/libMath.la \
	MonteCarlo/libMonteCarlo.la \
	Pricers/libPricers.la \
	Solvers1D/libSolvers1D.la \
	TermStructures/libTermStructures.la

libQuantLib_la_SOURCES = \
    calendar.cpp \
    dataformatters.cpp \
    date.cpp \
    scheduler.cpp \
    solver1d.cpp

lib_LTLIBRARIES = libQuantLib.la
