SUBDIRS = Calendars Currencies DayCounters FiniteDifferences Instruments \
	Math Patterns Pricers Solvers1D TermStructures MonteCarlo \
	Utilities

includedir = $(prefix)/include/QuantLib
include_HEADERS = calendar.h \
	config.h \
	array.h \
	currency.h \
	dataformatters.h \
	date.h \
	daycounter.h \
	deposit.h \
	discountfactor.h \
	expressiontemplates.h \
	forwardvolsurface.h \
	handle.h \
	history.h instrument.h \
	null.h \
	options.h qldefines.h \
	qlerrors.h quantlib.h \
	rate.h \
	solver1d.h spread.h \
	swaptionvolsurface.h \
	termstructure.h

install-data-hook:
	$(SED) -e 's/HAVE_CONFIG_H/QL_HAVE_CONFIG_H/;/#ifndef quantlib_defines_h/{G;s/$$/#define QL_HAVE_CONFIG_H/;}' $(includedir)/qldefines.h > .qldefines.h
	$(INSTALL_DATA) .qldefines.h $(includedir)/qldefines.h
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)
