
# $Id$
# $Source$
# $Log$
# Revision 1.12  2001/07/09 16:29:27  lballabio
# Some documentation and market element
#
# Revision 1.11  2001/07/05 15:57:22  lballabio
# Collected typedefs in a single file
#
# Revision 1.10  2001/05/31 08:56:40  lballabio
# Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
#
# Revision 1.9  2001/05/28 14:54:25  lballabio
# Deposit rates are always adjusted
#
# Revision 1.8  2001/05/24 12:52:01  nando
# smoothing #include xx.hpp
#

SUBDIRS = Calendars CashFlows DayCounters FiniteDifferences \
    Indexes Instruments Math MonteCarlo Patterns Pricers \
    Solvers1D TermStructures Utilities

EXTRA_DIST=config.ansi.hpp config.bcc.hpp config.decc.hpp \
	config.msvc.hpp config.mwcw.hpp

includedir = $(prefix)/include/ql
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
