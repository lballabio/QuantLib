
# $Id$
# $Source$

SUBDIRS = Calendars \
	DayCounters \
    FiniteDifferences \
	Math \
	MonteCarlo \
	Pricers \
    Solvers1D \
    TermStructures

EXTRA_DIST=makefile.mak

libQuantLib_la_LIBADD = ./Calendars/libCalendars.la \
	DayCounters/libDayCounters.la \
	FiniteDifferences/libFiniteDifferences.la \
	Math/libMath.la \
	MonteCarlo/libMonteCarlo.la \
	Pricers/libPricers.la \
	Solvers1D/libSolvers1D.la \
	TermStructures/libTermStructures.la


libQuantLib_la_SOURCES = calendar.cpp \
    dataformatters.cpp \
    date.cpp \
    ratehelper.cpp \
    solver1d.cpp

lib_LTLIBRARIES = libQuantLib.la
