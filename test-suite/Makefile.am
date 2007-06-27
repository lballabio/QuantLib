
QL_TESTS = \
	quantlibtestsuite.cpp \
	americanoption.hpp americanoption.cpp \
	array.hpp array.cpp \
	asianoptions.hpp asianoptions.cpp \
	assetswap.hpp assetswap.cpp \
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
	curvestates.hpp curvestates.cpp \
	dates.hpp dates.cpp \
	daycounters.hpp daycounters.cpp \
	digitalcoupon.hpp digitalcoupon.cpp \
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
	marketmodel_cms.hpp marketmodel_cms.cpp \
	marketmodel_smm.hpp marketmodel_smm.cpp \
	marketmodel_smmcapletalphacalibration.hpp marketmodel_smmcapletalphacalibration.cpp \
	marketmodel_smmcapletcalibration.hpp marketmodel_smmcapletcalibration.cpp \
	marketmodel_smmcaplethomocalibration.hpp marketmodel_smmcaplethomocalibration.cpp \
	matrices.hpp matrices.cpp \
	mclongstaffschwartzengine.hpp mclongstaffschwartzengine.cpp \
	mersennetwister.hpp mersennetwister.cpp \
	money.hpp money.cpp \
	operators.hpp operators.cpp \
	optimizers.hpp optimizers.cpp \
	period.hpp period.cpp \
	pathgenerator.hpp pathgenerator.cpp \
	piecewiseyieldcurve.hpp piecewiseyieldcurve.cpp \
	quantooption.hpp quantooption.cpp \
	quotes.hpp quotes.cpp \
	rangeaccrual.hpp rangeaccrual.cpp \
	riskstats.hpp riskstats.cpp \
	rngtraits.hpp rngtraits.cpp \
	rounding.hpp rounding.cpp \
	sampledcurve.hpp sampledcurve.cpp \
	shortratemodels.hpp shortratemodels.cpp \
	solvers.hpp solvers.cpp \
	stats.hpp stats.cpp \
	surface.hpp surface.cpp \
	swap.hpp swap.cpp \
	swapforwardmappings.hpp swapforwardmappings.cpp \
	swaption.hpp swaption.cpp \
	swaptionvolatilitycube.hpp swaptionvolatilitycube.cpp \
	swaptionvolatilitymatrix.hpp swaptionvolatilitymatrix.cpp \
	swaptionvolatilitystructures.hpp swaptionvolatilitystructures.cpp \
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
	rm -f $(distdir)/main.cpp

if BOOST_UNIT_TEST_FOUND

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} ${BOOST_UNIT_TEST_DEFINE}

noinst_LTLIBRARIES = libUnitMain.la
libUnitMain_la_SOURCES = main.cpp
libUnitMain_la_CXXFLAGS = ${BOOST_UNIT_TEST_MAIN_CXXFLAGS}

main.cpp: Makefile.am
	echo "#ifdef BOOST_TEST_DYN_LINK" > $@
	echo "#include <boost/test/unit_test.hpp>" >> $@
	echo "using namespace boost::unit_test;" >> $@
	echo "test_suite* init_unit_test_suite(int, char* []);" >> $@
	echo "bool init_function() {" >> $@
	echo "framework::master_test_suite().add(init_unit_test_suite(0,0));" >> $@
	echo "return true;" >> $@
	echo "}" >> $@
	echo "int main( int argc, char* argv[] ) {" >> $@
	echo "return ::boost::unit_test::unit_test_main( &init_function, argc, argv );" >> $@
	echo "}" >> $@
	echo "#endif" >> $@


if AUTO_BENCHMARK
bin_PROGRAMS = quantlib-test-suite quantlib-benchmark
else
bin_PROGRAMS = quantlib-test-suite
EXTRA_PROGRAMS = quantlib-benchmark
CLEANFILES = quantlib-benchmark
endif

quantlib_test_suite_SOURCES = ${QL_TESTS}
quantlib_test_suite_LDADD = libUnitMain.la ${top_builddir}/ql/libQuantLib.la
quantlib_test_suite_LDFLAGS = -l${BOOST_UNIT_TEST_LIB}

quantlib_benchmark_SOURCES = ${QL_BENCHMARKS}
quantlib_benchmark_LDADD = libUnitMain.la ${top_builddir}/ql/libQuantLib.la
quantlib_benchmark_LDFLAGS = -l${BOOST_UNIT_TEST_LIB}

TESTS = quantlib-test-suite
TESTS_ENVIRONMENT = BOOST_TEST_LOG_LEVEL=message

.PHONY: benchmark
benchmark: quantlib-benchmark
	BOOST_TEST_LOG_LEVEL=message ./quantlib-benchmark$(EXEEXT)

EXTRA_DIST = \
	README.txt \
	testsuite_vc7.vcproj \
	testsuite_vc8.vcproj \
	testsuite.dev

else

EXTRA_DIST = \
	${QL_TESTS} \
	quantlibbenchmark.cpp \
	README.txt \
	testsuite_vc7.vcproj \
	testsuite_vc8.vcproj \
	testsuite.dev

endif
