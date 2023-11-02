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

#include "everestoption.hpp"
#include "exchangerate.hpp"
#include "extendedtrees.hpp"
#include "extensibleoptions.hpp"
#include "fastfouriertransform.hpp"
#include "fdcev.hpp"
#include "fdcir.hpp"
#include "fdheston.hpp"
#include "fdmlinearop.hpp"
#include "fdsabr.hpp"
#include "fittedbonddiscountcurve.hpp"
#include "forwardoption.hpp"
#include "forwardrateagreement.hpp"
#include "functions.hpp"
#include "garch.hpp"
#include "gaussianquadratures.hpp"
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
#include "marketmodel_cms.hpp"
#include "marketmodel_smm.hpp"
#include "marketmodel_smmcapletalphacalibration.hpp"
#include "marketmodel_smmcapletcalibration.hpp"
#include "marketmodel_smmcaplethomocalibration.hpp"
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
#include "speedlevel.hpp"
#include "spreadoption.hpp"
#include "squarerootclvmodel.hpp"
#include "stats.hpp"
#include "subperiodcoupons.hpp"
#include "svivolatility.hpp"
#include "swap.hpp"
#include "swapforwardmappings.hpp"
#include "swaption.hpp"
#include "swaptionvolatilitycube.hpp"
#include "swaptionvolatilitymatrix.hpp"
#include "swingoption.hpp"
#include "termstructures.hpp"
#include "timegrid.hpp"
#include "timeseries.hpp"
#include "tqreigendecomposition.hpp"
#include "tracing.hpp"
#include "transformedgrid.hpp"
#include "twoassetbarrieroption.hpp"
#include "twoassetcorrelationoption.hpp"
#include "ultimateforwardtermstructure.hpp"
#include "utilities.hpp"
#include "variancegamma.hpp"
#include "varianceoption.hpp"
#include "varianceswaps.hpp"
#include "volatilitymodels.hpp"
#include "vpp.hpp"
#include "xoshiro256starstar.hpp"
#include "zabr.hpp"
#include "zerocouponswap.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>

using namespace boost::unit_test_framework;

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
    SpeedLevel speed = speed_level(argc, argv);

    auto* test = BOOST_TEST_SUITE("QuantLib test suite");

    test->add(CPISwapTest::suite());
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
    test->add(IntegralTest::suite());
    test->add(InterestRateTest::suite());
    test->add(InterpolationTest::suite(speed));
    test->add(JumpDiffusionTest::suite());
    test->add(LazyObjectTest::suite());
    test->add(LinearLeastSquaresRegressionTest::suite());
    test->add(LookbackOptionTest::suite(speed));
    test->add(LowDiscrepancyTest::suite());
    test->add(MargrabeOptionTest::suite());
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
    test->add(Xoshiro256StarStarTest::suite());
    test->add(ZeroCouponSwapTest::suite());

    // tests for experimental classes
    test->add(EverestOptionTest::suite());
    test->add(ExtendedTreesTest::suite());
    test->add(ExtensibleOptionsTest::suite());
    test->add(GaussianQuadraturesTest::experimental());
    test->add(HestonModelTest::experimental());
    test->add(HimalayaOptionTest::suite());
    test->add(InflationCPICapFloorTest::suite());
    test->add(InflationVolTest::suite());
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

    return test;
}
