
# $Id$
# $Source$
# $Log$
# Revision 1.14  2001/05/24 12:52:02  nando
# smoothing #include xx.hpp
#

SUBDIRS = Calendars \
	DayCounters \
    FiniteDifferences \
    Indexes \
	Math \
	MonteCarlo \
	Pricers \
    Solvers1D \
    TermStructures

EXTRA_DIST=makefile.mak

libQuantLib_la_LIBADD = ./Calendars/libCalendars.la \
	DayCounters/libDayCounters.la \
	FiniteDifferences/libFiniteDifferences.la \
	Indexes/libIndexes.la \
	Math/libMath.la \
	MonteCarlo/libMonteCarlo.la \
	Pricers/libPricers.la \
	Solvers1D/libSolvers1D.la \
	TermStructures/libTermStructures.la

libQuantLib_la_SOURCES = calendar.cpp \
    dataformatters.cpp \
    date.cpp \
    solver1d.cpp

lib_LTLIBRARIES = libQuantLib.la
