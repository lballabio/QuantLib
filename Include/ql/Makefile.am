
# $Id$
# $Source$

SUBDIRS = Calendars Currencies DayCounters FiniteDifferences Indexes \
    Instruments Math MonteCarlo Patterns Pricers Solvers1D \
    TermStructures Utilities

EXTRA_DIST=config.ansi.hpp config.bcc.hpp config.decc.hpp \
	config.msvc.hpp config.mwcw.hpp

includedir = $(prefix)/include/ql
include_HEADERS = \
	array.hpp \
	calendar.hpp \
	config.hpp \
	currency.hpp \
	dataformatters.hpp \
	date.hpp \
	daycounter.hpp \
	depositrate.hpp \
	discountfactor.hpp \
	expressiontemplates.hpp \
	forwardvolsurface.hpp \
	handle.hpp \
	history.hpp \
	instrument.hpp \
	null.hpp \
	options.hpp \
	qldefines.hpp \
	qlerrors.hpp \
	quantlib.hpp \
	rate.hpp \
	riskstatistics.hpp \
	solver1d.hpp spread.hpp \
	swaptionvolsurface.hpp \
	termstructure.hpp

install-data-hook:
	$(SED) -e 's/HAVE_CONFIG_H/QL_HAVE_CONFIG_H/;/#ifndef quantlib_defines_h/{G;s/$$/#define QL_HAVE_CONFIG_H/;}' $(includedir)/qldefines.hpp > .qldefines.hpp
	$(INSTALL_DATA) .qldefines.hpp $(includedir)/qldefines.hpp
depend:
	makedepend $(INCLUDES) -- $(CFLAGS) -- $(SOURCES)
