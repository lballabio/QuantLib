
# $Id$
# $Source$
# $Log$
# Revision 1.16  2001/05/31 10:10:08  lballabio
# Added new dirs in configure.in
#
# Revision 1.15  2001/05/31 08:56:40  lballabio
# Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
#
# Revision 1.14  2001/05/24 12:52:02  nando
# smoothing #include xx.hpp
#

SUBDIRS = Calendars \
    CashFlows \
    DayCounters \
    FiniteDifferences \
    Indexes \
    Instruments \
    Math \
    MonteCarlo \
    Pricers \
    Solvers1D \
    TermStructures

EXTRA_DIST=makefile.mak

libQuantLib_la_LIBADD = ./Calendars/libCalendars.la \
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

libQuantLib_la_SOURCES = calendar.cpp \
    dataformatters.cpp \
    date.cpp \
    scheduler.cpp \
    solver1d.cpp

lib_LTLIBRARIES = libQuantLib.la
