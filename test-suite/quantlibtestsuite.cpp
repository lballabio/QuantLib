/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/types.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <ql/version.hpp>

#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#include "paralleltestrunner.hpp"
#else
#include <boost/test/included/unit_test.hpp>
#endif

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif

#include "utilities.hpp"
#include "speedlevel.hpp"

#include "americanoption.hpp"
#include "andreasenhugevolatilityinterpl.hpp"
#include "amortizingbond.hpp"
#include "array.hpp"
#include "asianoptions.hpp"
#include "assetswap.hpp"
#include "autocovariances.hpp"
#include "barrieroption.hpp"
#include "basismodels.hpp"
#include "basisswapratehelpers.hpp"
#include "basketoption.hpp"
#include "batesmodel.hpp"
#include "bermudanswaption.hpp"
#include "binaryoption.hpp"
#include "blackdeltacalculator.hpp"
#include "blackformula.hpp"
#include "bonds.hpp"
#include "bondforward.hpp"
#include "brownianbridge.hpp"
#include "businessdayconventions.hpp"
#include "calendars.hpp"
#include "callablebonds.hpp"
#include "capfloor.hpp"
#include "capflooredcoupon.hpp"
#include "cashflows.hpp"
#include "catbonds.hpp"
#include "cdo.hpp"
#include "cdsoption.hpp"
#include "chooseroption.hpp"
#include "cliquetoption.hpp"
#include "cms.hpp"
#include "cmsspread.hpp"
#include "commodityunitofmeasure.hpp"
#include "compiledboostversion.hpp"
#include "compoundoption.hpp"
#include "convertiblebonds.hpp"
#include "covariance.hpp"
#include "creditdefaultswap.hpp"
#include "creditriskplus.hpp"
#include "crosscurrencyratehelpers.hpp"
#include "currency.hpp"
#include "curvestates.hpp"
#include "dates.hpp"
#include "daycounters.hpp"
#include "defaultprobabilitycurves.hpp"
#include "digitalcoupon.hpp"
#include "digitaloption.hpp"
#include "distributions.hpp"
#include "dividendoption.hpp"
#include "doublebarrieroption.hpp"
#include "doublebinaryoption.hpp"
#include "europeanoption.hpp"
#include "everestoption.hpp"
#include "equityindex.hpp"
#include "equitycashflow.hpp"
#include "equitytotalreturnswap.hpp"
#include "exchangerate.hpp"
#include "extendedtrees.hpp"
#include "extensibleoptions.hpp"
#include "fastfouriertransform.hpp"
#include "fdheston.hpp"
#include "fdcir.hpp"
#include "fdmlinearop.hpp"
#include "fdcev.hpp"
#include "fdsabr.hpp"
#include "fittedbonddiscountcurve.hpp"
#include "forwardoption.hpp"
#include "forwardrateagreement.hpp"
#include "functions.hpp"
#include "gaussianquadratures.hpp"
#include "garch.hpp"
#include "gjrgarchmodel.hpp"
#include "gsr.hpp"
#include "hestonmodel.hpp"
#include "hestonslvmodel.hpp"
#include "himalayaoption.hpp"
#include "hybridhestonhullwhiteprocess.hpp"
#include "indexes.hpp"
#include "inflation.hpp"
#include "inflationcapfloor.hpp"
#include "inflationcapflooredcoupon.hpp"
#include "inflationcpibond.hpp"
#include "inflationcpicapfloor.hpp"
#include "inflationcpiswap.hpp"
#include "inflationvolatility.hpp"
#include "instruments.hpp"
#include "integrals.hpp"
#include "interestrates.hpp"
#include "interpolations.hpp"
#include "jumpdiffusion.hpp"
#include "lazyobject.hpp"
#include "libormarketmodel.hpp"
#include "libormarketmodelprocess.hpp"
#include "linearleastsquaresregression.hpp"
#include "lookbackoptions.hpp"
#include "lowdiscrepancysequences.hpp"
#include "margrabeoption.hpp"
#include "marketmodel.hpp"
#include "marketmodel_smmcapletalphacalibration.hpp"
#include "marketmodel_smmcapletcalibration.hpp"
#include "marketmodel_smmcaplethomocalibration.hpp"
#include "marketmodel_smm.hpp"
#include "marketmodel_cms.hpp"
#include "markovfunctional.hpp"
#include "matrices.hpp"
#include "mclongstaffschwartzengine.hpp"
#include "mersennetwister.hpp"
#include "money.hpp"
#include "noarbsabr.hpp"
#include "normalclvmodel.hpp"
#include "nthorderderivativeop.hpp"
#include "nthtodefault.hpp"
#include "numericaldifferentiation.hpp"
#include "observable.hpp"
#include "ode.hpp"
#include "operators.hpp"
#include "optimizers.hpp"
#include "optionletstripper.hpp"
#include "overnightindexedcoupon.hpp"
#include "overnightindexedswap.hpp"
#include "pagodaoption.hpp"
#include "partialtimebarrieroption.hpp"
#include "pathgenerator.hpp"
#include "period.hpp"
#include "piecewiseyieldcurve.hpp"
#include "piecewisezerospreadedtermstructure.hpp"
#include "quantooption.hpp"
#include "quotes.hpp"
#include "rangeaccrual.hpp"
#include "riskneutraldensitycalculator.hpp"
#include "riskstats.hpp"
#include "rngtraits.hpp"
#include "rounding.hpp"
#include "sampledcurve.hpp"
#include "schedule.hpp"
#include "settings.hpp"
#include "shortratemodels.hpp"
#include "sofrfutures.hpp"
#include "solvers.hpp"
#include "spreadoption.hpp"
#include "squarerootclvmodel.hpp"
#include "swingoption.hpp"
#include "stats.hpp"
#include "subperiodcoupons.hpp"
#include "svivolatility.hpp"
#include "swap.hpp"
#include "swapforwardmappings.hpp"
#include "swaption.hpp"
#include "swaptionvolatilitycube.hpp"
#include "swaptionvolatilitymatrix.hpp"
#include "termstructures.hpp"
#include "timegrid.hpp"
#include "timeseries.hpp"
#include "tqreigendecomposition.hpp"
#include "tracing.hpp"
#include "transformedgrid.hpp"
#include "twoassetbarrieroption.hpp"
#include "twoassetcorrelationoption.hpp"
#include "ultimateforwardtermstructure.hpp"
#include "variancegamma.hpp"
#include "varianceoption.hpp"
#include "varianceswaps.hpp"
#include "volatilitymodels.hpp"
#include "vpp.hpp"
#include "zabr.hpp"
#include "zerocouponswap.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>

using namespace boost::unit_test_framework;

namespace {

    decltype(std::chrono::steady_clock::now()) start;

    void startTimer() {
        start = std::chrono::steady_clock::now();
    }

    void stopTimer() {
        auto stop = std::chrono::steady_clock::now();

        double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() * 1e-3;
        int hours = int(seconds/3600);
        seconds -= hours * 3600;
        int minutes = int(seconds/60);
        seconds -= minutes * 60;

        std::cout << "\nTests completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;
    }

    void configure(QuantLib::Date evaluationDate) {
        /* if needed, a subset of the lines below can be
           uncommented and/or changed to run the test suite with a
           different configuration. In the future, we'll need a
           mechanism that doesn't force us to recompile (possibly a
           couple of command-line flags for the test suite?)
        */

        // QuantLib::Settings::instance().includeReferenceDateCashFlows() = true;
        // QuantLib::Settings::instance().includeTodaysCashFlows() = boost::none;

        QuantLib::Settings::instance().evaluationDate() = evaluationDate;
    }

}

QuantLib::Date evaluation_date(int argc, char** argv) {
    /*! Dead simple parser:
        - passing --date=YYYY-MM-DD causes the test suite to run on
          that date;
        - passing --date=today causes it to run on today's date;
        - passing nothing causes it to run on a known date for which
          there should be no date-dependent errors as far as we know.

        Dates that should eventually be checked include:
        - 2015-08-29 causes three tests to fail;
        - 2016-02-29 causes two tests to fail.
    */

    QuantLib::Date knownGoodDefault =
        QuantLib::Date(16, QuantLib::September, 2015);

    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--date=today")
            return QuantLib::Date::todaysDate();
        else if (arg.substr(0, 7) == "--date=")
            return QuantLib::DateParser::parseISO(arg.substr(7));
    }
    return knownGoodDefault;
}


SpeedLevel speed_level(int argc, char** argv) {
    /*! Again, dead simple parser:
        - passing --slow causes all tests to be run;
        - passing --fast causes most tests to be run, except the slowest;
        - passing --faster causes only the faster tests to be run;
        - passing nothing is the same as --slow
    */

    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--slow")
            return Slow;
        else if (arg == "--fast")
            return Fast;
        else if (arg == "--faster")
            return Faster;
    }
    return Slow;
}


test_suite* init_unit_test_suite(int, char* []) {

    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    configure(evaluation_date(argc, argv));
    SpeedLevel speed = speed_level(argc, argv);

    const QuantLib::Settings& settings = QuantLib::Settings::instance();
    std::ostringstream header;
    header <<
        " Testing "
        "QuantLib " QL_VERSION
        "\n  QL_EXTRA_SAFETY_CHECKS "
        #ifdef QL_EXTRA_SAFETY_CHECKS
        "  defined"
        #else
        "undefined"
        #endif
        "\n  QL_USE_INDEXED_COUPON "
        #ifdef QL_USE_INDEXED_COUPON
        "   defined"
        #else
        " undefined"
        #endif
        "\n"
           << "evaluation date is " << settings.evaluationDate() << ",\n"
           << (settings.includeReferenceDateEvents()
               ? "reference date events are included,\n"
               : "reference date events are excluded,\n")
           << (settings.includeTodaysCashFlows() == boost::none ?
               "" : (*settings.includeTodaysCashFlows() ?
                     "today's cashflows are included,\n"
                     : "today's cashflows are excluded,\n"))
           << (settings.enforcesTodaysHistoricFixings()
               ? "today's historic fixings are enforced."
               : "today's historic fixings are not enforced.")
           << "\nRunning "
           << (speed == Faster ? "faster" :
               (speed == Fast ?   "fast" : "all"))
           << " tests.";

    std::string rule = std::string(41, '=');

    BOOST_TEST_MESSAGE(rule);
    BOOST_TEST_MESSAGE(header.str());
    BOOST_TEST_MESSAGE(rule);
    auto* test = BOOST_TEST_SUITE("QuantLib test suite");

    test->add(QUANTLIB_TEST_CASE(startTimer));

    test->add(AmericanOptionTest::suite(speed));
    test->add(AmortizingBondTest::suite());
    test->add(AndreasenHugeVolatilityInterplTest::suite(speed));
    test->add(ArrayTest::suite());
    test->add(AsianOptionTest::suite(speed));
    test->add(AssetSwapTest::suite()); // fails with QL_USE_INDEXED_COUPON
    test->add(AutocovariancesTest::suite());
    test->add(BarrierOptionTest::suite());
    test->add(BasketOptionTest::suite(speed));
    test->add(BatesModelTest::suite());
    test->add(BermudanSwaptionTest::suite(speed));
    test->add(BinaryOptionTest::suite());
    test->add(BlackFormulaTest::suite());
    test->add(BondTest::suite());
    test->add(BondForwardTest::suite());
    test->add(BrownianBridgeTest::suite());
    test->add(BusinessDayConventionTest::suite());
    test->add(CalendarTest::suite());
    test->add(CapFloorTest::suite());
    test->add(CapFlooredCouponTest::suite());
    test->add(CashFlowsTest::suite());
    test->add(CliquetOptionTest::suite());
    test->add(CmsTest::suite());
    test->add(ConvertibleBondTest::suite());
    test->add(CovarianceTest::suite());
    test->add(CPISwapTest::suite());
    test->add(CreditDefaultSwapTest::suite());
    test->add(CrossCurrencyRateHelpersTest::suite());
    test->add(CurrencyTest::suite());
    test->add(CurveStatesTest::suite());
    test->add(DateTest::suite(speed));
    test->add(DayCounterTest::suite());
    test->add(DefaultProbabilityCurveTest::suite());
    test->add(DigitalCouponTest::suite()); // might fail with QL_USE_INDEXED_COUPON
    test->add(DigitalOptionTest::suite());
    test->add(DistributionTest::suite(speed));
    test->add(DividendOptionTest::suite());
    test->add(EquityIndexTest::suite());
    test->add(EquityCashFlowTest::suite());
    test->add(EquityTotalReturnSwapTest::suite());
    test->add(EuropeanOptionTest::suite());
    test->add(ExchangeRateTest::suite());
    test->add(FastFourierTransformTest::suite());
    test->add(FdHestonTest::suite(speed));
    test->add(FdmLinearOpTest::suite(speed));
    test->add(FdCevTest::suite(speed));
    test->add(FdCIRTest::suite(speed));
    test->add(FdSabrTest::suite(speed));
    test->add(FittedBondDiscountCurveTest::suite());
    test->add(ForwardOptionTest::suite(speed));
    test->add(ForwardRateAgreementTest::suite());
    test->add(FunctionsTest::suite());
    test->add(GARCHTest::suite());
    test->add(GaussianQuadraturesTest::suite());
    test->add(GJRGARCHModelTest::suite(speed));
    test->add(GsrTest::suite());
    test->add(HestonModelTest::suite(speed));
    test->add(HestonSLVModelTest::suite(speed));
    test->add(HybridHestonHullWhiteProcessTest::suite(speed));
    test->add(IndexTest::suite());
    test->add(InflationTest::suite());
    test->add(InflationCapFloorTest::suite());
    test->add(InflationCapFlooredCouponTest::suite());
    test->add(InflationCPIBondTest::suite());
    test->add(InstrumentTest::suite());
    test->add(IntegralTest::suite());
    test->add(InterestRateTest::suite());
    test->add(InterpolationTest::suite(speed));
    test->add(JumpDiffusionTest::suite());
    test->add(LazyObjectTest::suite());
    test->add(LinearLeastSquaresRegressionTest::suite());
    test->add(LookbackOptionTest::suite(speed));
    test->add(LowDiscrepancyTest::suite());
    test->add(MarketModelTest::suite(speed));
    test->add(MarketModelCmsTest::suite(speed));
    test->add(MarketModelSmmTest::suite(speed));
    test->add(MarketModelSmmCapletAlphaCalibrationTest::suite());
    test->add(MarketModelSmmCapletCalibrationTest::suite());
    test->add(MarketModelSmmCapletHomoCalibrationTest::suite());
    test->add(MarkovFunctionalTest::suite(speed));
    test->add(MatricesTest::suite());
    test->add(MCLongstaffSchwartzEngineTest::suite(speed));
    test->add(MersenneTwisterTest::suite());
    test->add(MoneyTest::suite());
    test->add(NumericalDifferentiationTest::suite());
    test->add(NthOrderDerivativeOpTest::suite(speed));
    test->add(ObservableTest::suite());
    test->add(OdeTest::suite());
    test->add(OperatorTest::suite());
    test->add(OptimizersTest::suite(speed));
    test->add(OptionletStripperTest::suite());
    test->add(OvernightIndexedCouponTest::suite());
    test->add(OvernightIndexedSwapTest::suite());
    test->add(PathGeneratorTest::suite());
    test->add(PeriodTest::suite());
    test->add(PiecewiseYieldCurveTest::suite());
    test->add(PiecewiseZeroSpreadedTermStructureTest::suite());
    test->add(QuantoOptionTest::suite());
    test->add(QuoteTest::suite());
    test->add(RangeAccrualTest::suite());
    test->add(RiskStatisticsTest::suite());
    test->add(RngTraitsTest::suite());
    test->add(RoundingTest::suite());
    test->add(SampledCurveTest::suite());
    test->add(ScheduleTest::suite());
    test->add(SettingsTest::suite());
    test->add(ShortRateModelTest::suite(speed)); // fails with QL_USE_INDEXED_COUPON
    test->add(SofrFuturesTest::suite());
    test->add(Solver1DTest::suite());
    test->add(StatisticsTest::suite());
    test->add(SubPeriodsCouponTest::suite());
    test->add(SwapTest::suite());
    test->add(SwapForwardMappingsTest::suite());
    test->add(SwaptionTest::suite(speed));
    test->add(SwaptionVolatilityCubeTest::suite());
    test->add(SwaptionVolatilityMatrixTest::suite());
    test->add(TermStructureTest::suite());
    test->add(TimeGridTest::suite());
    test->add(TimeSeriesTest::suite());
    test->add(TqrEigenDecompositionTest::suite());
    test->add(TracingTest::suite());
    test->add(TransformedGridTest::suite());
    test->add(UltimateForwardTermStructureTest::suite());
    test->add(VarianceSwapTest::suite());
    test->add(VolatilityModelsTest::suite());
    test->add(ZeroCouponSwapTest::suite());

    // tests for experimental classes
    test->add(AsianOptionTest::experimental(speed));
    test->add(BasismodelsTest::suite());
    test->add(BasisSwapRateHelpersTest::suite());
    test->add(BarrierOptionTest::experimental());
    test->add(DoubleBarrierOptionTest::experimental(speed));
    test->add(BlackDeltaCalculatorTest::suite());
    test->add(CallableBondTest::suite());
    test->add(CatBondTest::suite());
    test->add(CdoTest::suite(speed));
    test->add(CdsOptionTest::suite());
    test->add(ChooserOptionTest::suite());
    test->add(CmsSpreadTest::suite());
    test->add(CommodityUnitOfMeasureTest::suite());
    test->add(CompiledBoostVersionTest::suite());
    test->add(CompoundOptionTest::suite());
    test->add(CreditRiskPlusTest::suite());
    test->add(DoubleBarrierOptionTest::suite(speed));
    test->add(DoubleBinaryOptionTest::suite());
    test->add(EuropeanOptionTest::experimental());
    test->add(EverestOptionTest::suite());
    test->add(ExtendedTreesTest::suite());
    test->add(ExtensibleOptionsTest::suite());
    test->add(GaussianQuadraturesTest::experimental());
    test->add(HestonModelTest::experimental());
    test->add(HimalayaOptionTest::suite());
    test->add(InflationCPICapFloorTest::suite());
    test->add(InflationVolTest::suite());
    test->add(MargrabeOptionTest::suite());
    test->add(NoArbSabrTest::suite());
    test->add(NormalCLVModelTest::experimental(speed));
    test->add(NthToDefaultTest::suite(speed));
    test->add(PagodaOptionTest::suite());
    test->add(PartialTimeBarrierOptionTest::suite());
    test->add(QuantoOptionTest::experimental());
    test->add(RiskNeutralDensityCalculatorTest::experimental(speed));
    test->add(SpreadOptionTest::suite());
    test->add(SquareRootCLVModelTest::experimental());
    test->add(SviVolatilityTest::experimental());
    test->add(SwingOptionTest::suite(speed));
    test->add(TwoAssetBarrierOptionTest::suite());
    test->add(TwoAssetCorrelationOptionTest::suite());
    test->add(VarianceGammaTest::suite());
    test->add(VarianceOptionTest::suite());
    test->add(VPPTest::suite(speed));
    test->add(ZabrTest::suite(speed));

    // tests for deprecated classes
    test->add(LiborMarketModelTest::suite(speed));
    test->add(LiborMarketModelProcessTest::suite(speed));

    test->add(QUANTLIB_TEST_CASE(stopTimer));

    return test;
}
