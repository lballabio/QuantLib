INCLUDES = -I$(top_srcdir)/Include -I$(top_srcdir)/Include/Math

SUBDIRS = FiniteDifferences Calendars Solvers1D TermStructures \
	DayCounters Math Pricers MonteCarlo

libQuantLib_la_LIBADD = ./Calendars/libCalendars.la \
	DayCounters/libDayCounters.la \
	FiniteDifferences/libFiniteDifferences.la \
	Math/libMath.la \
	MonteCarlo/libMonteCarlo.la \
	Pricers/libPricers.la \
	Solvers1D/libSolvers1D.la \
	TermStructures/libTermStructures.la


libQuantLib_la_SOURCES = calendar.cpp dataformatters.cpp date.cpp \
    solver1d.cpp risktool.cpp

lib_LTLIBRARIES = libQuantLib.la
