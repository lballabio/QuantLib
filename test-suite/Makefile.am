
QL_TESTS = \
	quantlibtestsuite.cpp \
	americanoption.hpp americanoption.cpp \
	asianoptions.hpp asianoptions.cpp \
	barrieroption.hpp barrieroption.cpp \
	basketoption.hpp basketoption.cpp \
	calendars.hpp calendars.cpp \
	capfloor.hpp capfloor.cpp \
	cliquetoption.hpp cliquetoption.cpp \
	compoundforward.hpp compoundforward.cpp \
	covariance.hpp covariance.cpp \
	dates.hpp dates.cpp \
	daycounters.hpp daycounters.cpp \
	digitaloption.hpp digitaloption.cpp \
	distributions.hpp distributions.cpp \
	dividendeuropeanoption.hpp dividendeuropeanoption.cpp \
	europeanoption.hpp europeanoption.cpp \
	factorial.hpp factorial.cpp \
	forwardoption.hpp forwardoption.cpp \
	instruments.hpp instruments.cpp \
	integrals.hpp integrals.cpp \
	interpolations.hpp interpolations.cpp \
	jumpdiffusion.hpp jumpdiffusion.cpp \
	lowdiscrepancysequences.hpp lowdiscrepancysequences.cpp \
	matrices.hpp matrices.cpp \
	mersennetwister.hpp mersennetwister.cpp \
	operators.hpp operators.cpp \
	piecewiseflatforward.hpp piecewiseflatforward.cpp \
	quantooption.hpp quantooption.cpp \
	quotes.hpp quotes.cpp \
	riskstats.hpp riskstats.cpp \
	solvers.hpp solvers.cpp \
	stats.hpp stats.cpp \
	swap.hpp swap.cpp \
	swaption.hpp swaption.cpp \
	termstructures.hpp termstructures.cpp \
	old_pricers.hpp old_pricers.cpp \
	utilities.hpp utilities.cpp

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

bin_PROGRAMS = quantlib-test-suite

quantlib_test_suite_SOURCES = ${QL_TESTS}
quantlib_test_suite_LDADD = ../ql/libQuantLib.la
quantlib_test_suite_LDFLAGS = -lboost_unit_test_framework

TESTS = quantlib-test-suite

TESTS_ENVIRONMENT = BOOST_TEST_LOG_LEVEL=messages

EXTRA_DIST = README.txt testsuite.dsp testsuite.vcproj makefile.mak
