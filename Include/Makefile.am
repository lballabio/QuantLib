SUBDIRS = Calendars Currencies DayCounters FiniteDifferences Instruments \
	Math Patterns Pricers Solvers1D TermStructures MonteCarlo \
	Utilities

EXTRA_DIST=config.ansi.h config.bcc.h config.decc.h \
	config.msvc.h config.mwcw.h

includedir = $(prefix)/include/QuantLib
include_HEADERS = \
	array.h \
	calendar.h \
	config.h \
	currency.h \
	dataformatters.h \
	date.h \
	daycounter.h \
	depositrate.h \
	discountfactor.h \
	expressiontemplates.h \
	forwardvolsurface.h \
	handle.h \
	history.h \
	instrument.h \
	null.h \
	options.h \
	qldefines.h \
	qlerrors.h \
	quantlib.h \
	rate.h \
	ratehelper.h \
	riskstatistics.h \
	solver1d.h spread.h \
	swaptionvolsurface.h \
	termstructure.h

install-data-hook:
	$(SED) -e 's/HAVE_CONFIG_H/QL_HAVE_CONFIG_H/;/#ifndef quantlib_defines_h/{G;s/$$/#define QL_HAVE_CONFIG_H/;}' $(includedir)/qldefines.h > .qldefines.h
	$(INSTALL_DATA) .qldefines.h $(includedir)/qldefines.h
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)
