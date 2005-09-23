
QL_TESTS = \
	quantlibtestsuite.cpp \
	americanoption.hpp americanoption.cpp \
	array.hpp array.cpp \
	asianoptions.hpp asianoptions.cpp \
	barrieroption.hpp barrieroption.cpp \
	basketoption.hpp basketoption.cpp \
	batesmodel.hpp batesmodel.cpp \
	bermudanswaption.hpp bermudanswaption.cpp \
	bonds.hpp bonds.cpp \
	calendars.hpp calendars.cpp \
	capfloor.hpp capfloor.cpp \
	cliquetoption.hpp cliquetoption.cpp \
	compoundforward.hpp compoundforward.cpp \
	covariance.hpp covariance.cpp \
	dates.hpp dates.cpp \
	daycounters.hpp daycounters.cpp \
	digitaloption.hpp digitaloption.cpp \
	distributions.hpp distributions.cpp \
	dividendoption.hpp dividendoption.cpp \
	europeanoption.hpp europeanoption.cpp \
	exchangerate.hpp exchangerate.cpp \
	factorial.hpp factorial.cpp \
	forwardoption.hpp forwardoption.cpp \
	gaussianquadratures.hpp gaussianquadratures.cpp \
	hestonmodel.hpp hestonmodel.cpp \
	instruments.hpp instruments.cpp \
	integrals.hpp integrals.cpp \
	interestrates.hpp interestrates.cpp \
	interpolations.hpp interpolations.cpp \
	jumpdiffusion.hpp jumpdiffusion.cpp \
	libormarketmodelprocess.hpp libormarketmodelprocess.cpp \
	lowdiscrepancysequences.hpp lowdiscrepancysequences.cpp \
	matrices.hpp matrices.cpp \
	mersennetwister.hpp mersennetwister.cpp \
	money.hpp money.cpp \
	operators.hpp operators.cpp \
	pathgenerator.hpp pathgenerator.cpp \
	piecewiseflatforward.hpp piecewiseflatforward.cpp \
	piecewiseyieldcurve.hpp piecewiseyieldcurve.cpp \
	quantooption.hpp quantooption.cpp \
	quotes.hpp quotes.cpp \
	riskstats.hpp riskstats.cpp \
	rngtraits.hpp rngtraits.cpp \
	rounding.hpp rounding.cpp \
	shortratemodels.hpp shortratemodels.cpp \
	solvers.hpp solvers.cpp \
	stats.hpp stats.cpp \
	swap.hpp swap.cpp \
	swaption.hpp swaption.cpp \
	termstructures.hpp termstructures.cpp \
	tqreigendecomposition.hpp tqreigendecomposition.cpp \
	tracing.hpp tracing.cpp \
	old_pricers.hpp old_pricers.cpp \
	utilities.hpp utilities.cpp

dist-hook:
	mkdir -p $(distdir)/build
	mkdir -p $(distdir)/bin
	cp -p $(srcdir)/bin/runtest.bat $(distdir)/bin

if BOOST_UNIT_TEST_FOUND

AM_CPPFLAGS = \
	-I${top_srcdir} -I${top_srcdir}/functions -I${top_builddir} \
	${BOOST_UNIT_TEST_DEFINE}

bin_PROGRAMS = quantlib-test-suite

quantlib_test_suite_SOURCES = ${QL_TESTS}
quantlib_test_suite_LDADD = \
	${top_builddir}/ql/libQuantLib.la \
	${top_builddir}/functions/ql/Functions/libQuantLibFunctions.la
quantlib_test_suite_LDFLAGS = -l${BOOST_UNIT_TEST_LIB}

TESTS = quantlib-test-suite

TESTS_ENVIRONMENT = BOOST_TEST_LOG_LEVEL=message

EXTRA_DIST = \
	README.txt \
	testsuite.dsp \
	testsuite.vcproj \
	testsuite_vc8.vcproj \
	testsuite.dev

else

EXTRA_DIST = \
	${QL_TESTS} \
	README.txt \
	testsuite.dsp \
	testsuite.vcproj \
	testsuite_vc8.vcproj \
	testsuite.dev

endif
