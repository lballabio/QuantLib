
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
	brownianbridge.hpp brownianbridge.cpp \
	calendars.hpp calendars.cpp \
	capfloor.hpp capfloor.cpp \
	capflooredcoupon.hpp capflooredcoupon.cpp \
	capstripper.hpp capstripper.cpp \
	cliquetoption.hpp cliquetoption.cpp \
	cms.hpp cms.cpp \
	compoundforward.hpp compoundforward.cpp \
	convertiblebonds.hpp convertiblebonds.cpp \
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
	libormarketmodel.hpp libormarketmodel.cpp \
	libormarketmodelprocess.hpp libormarketmodelprocess.cpp \
	linearleastsquaresregression.hpp linearleastsquaresregression.cpp \
	lookbackoptions.hpp lookbackoptions.cpp \
	lowdiscrepancysequences.hpp lowdiscrepancysequences.cpp \
	marketmodel.hpp marketmodel.cpp \
	matrices.hpp matrices.cpp \
	mclongstaffschwartzengine.hpp mclongstaffschwartzengine.cpp \
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
	sampledcurve.hpp sampledcurve.cpp \
	shortratemodels.hpp shortratemodels.cpp \
	solvers.hpp solvers.cpp \
	stats.hpp stats.cpp \
	surface.hpp surface.cpp \
	swap.hpp swap.cpp \
	swaption.hpp swaption.cpp \
	swaptionvolatilitymatrix.hpp swaptionvolatilitymatrix.cpp \
	swaptionvolatilitycube.hpp swaptionvolatilitycube.cpp \
	termstructures.hpp termstructures.cpp \
	timeseries.hpp timeseries.cpp \
	transformedgrid.hpp transformedgrid.cpp \
	tqreigendecomposition.hpp tqreigendecomposition.cpp \
	tracing.hpp tracing.cpp \
	old_pricers.hpp old_pricers.cpp \
	utilities.hpp utilities.cpp \
	varianceswaps.hpp varianceswaps.cpp \
	volatilitymodels.hpp volatilitymodels.cpp

QL_BENCHMARKS = \
	quantlibbenchmark.cpp \
	americanoption.hpp americanoption.cpp \
	asianoptions.hpp asianoptions.cpp \
	barrieroption.hpp barrieroption.cpp \
	basketoption.hpp basketoption.cpp \
	batesmodel.hpp batesmodel.cpp \
	digitaloption.hpp digitaloption.cpp \
	dividendoption.hpp dividendoption.cpp \
	europeanoption.hpp europeanoption.cpp \
	hestonmodel.hpp hestonmodel.cpp \
	jumpdiffusion.hpp jumpdiffusion.cpp \
	libormarketmodel.hpp libormarketmodel.cpp \
	libormarketmodelprocess.hpp libormarketmodelprocess.cpp \
	lowdiscrepancysequences.hpp lowdiscrepancysequences.cpp \
	quantooption.hpp quantooption.cpp \
	riskstats.hpp riskstats.cpp \
	shortratemodels.hpp shortratemodels.cpp \
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

if AUTO_BENCHMARK
bin_PROGRAMS = quantlib-test-suite quantlib-benchmark
else
bin_PROGRAMS = quantlib-test-suite
EXTRA_PROGRAMS = quantlib-benchmark
CLEANFILES = quantlib-benchmark
endif

quantlib_test_suite_SOURCES = ${QL_TESTS}
quantlib_test_suite_LDADD = ${top_builddir}/ql/libQuantLib.la
quantlib_test_suite_LDFLAGS = -l${BOOST_UNIT_TEST_LIB}

quantlib_benchmark_SOURCES = ${QL_BENCHMARKS}
quantlib_benchmark_LDADD = ${top_builddir}/ql/libQuantLib.la
quantlib_benchmark_LDFLAGS = -l${BOOST_UNIT_TEST_LIB}

TESTS = quantlib-test-suite
TESTS_ENVIRONMENT = BOOST_TEST_LOG_LEVEL=message

.PHONY: benchmark
benchmark: quantlib-benchmark
	BOOST_TEST_LOG_LEVEL=message ./quantlib-benchmark$(EXEEXT)

EXTRA_DIST = \
	README.txt \
	testsuite.dsp \
	testsuite.vcproj \
	testsuite_vc8.vcproj \
	testsuite.dev \
	makefile.mak

else

EXTRA_DIST = \
	${QL_TESTS} \
	quantlibbenchmark.cpp \
	README.txt \
	testsuite.dsp \
	testsuite.vcproj \
	testsuite_vc8.vcproj \
	testsuite.dev \
	makefile.mak

endif
