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
#include <ql/version.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#  define BOOST_LIB_NAME boost_unit_test_framework
#  include <boost/config/auto_link.hpp>
#  undef BOOST_LIB_NAME

/* uncomment the following lines to unmask floating-point exceptions.
   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
*/
//#  include <float.h>
//   namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }

#endif
#include "utilities.hpp"

#include "americanoption.hpp"
#include "array.hpp"
#include "asianoptions.hpp"
#include "assetswap.hpp"
#include "autocovariances.hpp"
#include "barrieroption.hpp"
#include "basketoption.hpp"
#include "batesmodel.hpp"
#include "bermudanswaption.hpp"
#include "blackdeltacalculator.hpp"
#include "bonds.hpp"
#include "brownianbridge.hpp"
#include "calendars.hpp"
#include "capfloor.hpp"
#include "capflooredcoupon.hpp"
#include "cashflows.hpp"
#include "cdo.hpp"
#include "cdsoption.hpp"
#include "chooseroption.hpp"
#include "cliquetoption.hpp"
#include "cms.hpp"
#include "commodityunitofmeasure.hpp"
#include "compoundoption.hpp"
#include "convertiblebonds.hpp"
#include "covariance.hpp"
#include "creditdefaultswap.hpp"
#include "curvestates.hpp"
#include "dates.hpp"
#include "daycounters.hpp"
#include "defaultprobabilitycurves.hpp"
#include "digitalcoupon.hpp"
#include "digitaloption.hpp"
#include "distributions.hpp"
#include "dividendoption.hpp"
#include "europeanoption.hpp"
#include "everestoption.hpp"
#include "exchangerate.hpp"
#include "extendedtrees.hpp"
#include "factorial.hpp"
#include "fastfouriertransform.hpp"
#include "fdheston.hpp"
#include "fdmlinearop.hpp"
#include "forwardoption.hpp"
#include "gaussianquadratures.hpp"
#include "gjrgarchmodel.hpp"
#include "hestonmodel.hpp"
#include "himalayaoption.hpp"
#include "hybridhestonhullwhiteprocess.hpp"
#include "inflation.hpp"
#include "inflationcapfloor.hpp"
#include "inflationcapflooredcoupon.hpp"
#include "inflationvolatility.hpp"
#include "instruments.hpp"
#include "integrals.hpp"
#include "interestrates.hpp"
#include "interpolations.hpp"
#include "libormarketmodel.hpp"
#include "libormarketmodelprocess.hpp"
#include "linearleastsquaresregression.hpp"
#include "jumpdiffusion.hpp"
#include "lookbackoptions.hpp"
#include "lowdiscrepancysequences.hpp"
#include "margrabeoption.hpp"
#include "marketmodel.hpp"
#include "marketmodel_smmcapletalphacalibration.hpp"
#include "marketmodel_smmcapletcalibration.hpp"
#include "marketmodel_smmcaplethomocalibration.hpp"
#include "marketmodel_smm.hpp"
#include "marketmodel_cms.hpp"
#include "matrices.hpp"
#include "mclongstaffschwartzengine.hpp"
#include "mersennetwister.hpp"
#include "money.hpp"
#include "nthtodefault.hpp"
#include "operators.hpp"
#include "optimizers.hpp"
#include "optionletstripper.hpp"
#include "overnightindexedswap.hpp"
#include "pagodaoption.hpp"
#include "pathgenerator.hpp"
#include "period.hpp"
#include "piecewiseyieldcurve.hpp"
#include "quantooption.hpp"
#include "quotes.hpp"
#include "riskstats.hpp"
#include "rngtraits.hpp"
#include "rounding.hpp"
#include "sampledcurve.hpp"
#include "shortratemodels.hpp"
#include "solvers.hpp"
#include "surface.hpp"
#include "stats.hpp"
#include "swap.hpp"
#include "swapforwardmappings.hpp"
#include "swaption.hpp"
#include "swaptionvolatilitycube.hpp"
#include "termstructures.hpp"
#include "timeseries.hpp"
#include "tqreigendecomposition.hpp"
#include "tracing.hpp"
#include "transformedgrid.hpp"
#include "variancegamma.hpp"
#include "varianceoption.hpp"
#include "varianceswaps.hpp"
#include "volatilitymodels.hpp"
#include "swaptionvolatilitymatrix.hpp"

#include <iostream>
#include <iomanip>

using namespace boost::unit_test_framework;

namespace {

    boost::timer t;

    void startTimer() { t.restart(); }
    void stopTimer() {
        double seconds = t.elapsed();
        int hours = int(seconds/3600);
        seconds -= hours * 3600;
        int minutes = int(seconds/60);
        seconds -= minutes * 60;
        std::cout << " \nTests completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;
    }

    void configure() {
        /* if needed, either or both the lines below can be
           uncommented and/or changed to run the test suite with a
           different configuration. In the future, we'll need a
           mechanism that doesn't force us to recompile (possibly a
           couple of command-line flags for the test suite?)
        */

        //QuantLib::Settings::instance().includeReferenceDateCashFlows() = true;
        //QuantLib::Settings::instance().includeTodaysCashFlows() = boost::none;
    }

}

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif

test_suite* init_unit_test_suite(int, char* []) {

    std::string header = "Testing "
                         #ifdef BOOST_MSVC
                         QL_LIB_NAME
                         #else
                         "QuantLib " QL_VERSION
                         #endif
                         #ifdef QL_DISABLE_DEPRECATED
                         " (deprecated code disabled)"
                         #endif
                         ;
    std::string rule = std::string(header.length(),'=');

    BOOST_MESSAGE(rule);
    BOOST_MESSAGE(header);
    BOOST_MESSAGE(rule);
    test_suite* test = BOOST_TEST_SUITE("QuantLib test suite");

    test->add(QUANTLIB_TEST_CASE(startTimer));
    test->add(QUANTLIB_TEST_CASE(configure));

    test->add(AmericanOptionTest::suite());
    test->add(ArrayTest::suite());
    test->add(AsianOptionTest::suite());
    test->add(AssetSwapTest::suite()); // fails with QL_USE_INDEXED_COUPON
    test->add(AutocovariancesTest::suite());
    test->add(BarrierOptionTest::suite());
    test->add(BasketOptionTest::suite());
    test->add(BatesModelTest::suite());
    test->add(BermudanSwaptionTest::suite());
    test->add(BlackDeltaCalculatorTest::suite());
    test->add(BondTest::suite());
    test->add(BrownianBridgeTest::suite());
    test->add(CalendarTest::suite());
    test->add(CapFloorTest::suite());
    test->add(CapFlooredCouponTest::suite());
    test->add(CashFlowsTest::suite());
    test->add(CdsOptionTest::suite());
    test->add(CdoTest::suite());
    test->add(ChooserOptionTest::suite());
    test->add(CliquetOptionTest::suite());
    test->add(CmsTest::suite());
    test->add(CommodityUnitOfMeasureTest::suite());
    test->add(CompoundOptionTest::suite());
    test->add(ConvertibleBondTest::suite());
    test->add(CovarianceTest::suite());
    test->add(CreditDefaultSwapTest::suite());
    test->add(CurveStatesTest::suite());
    test->add(DateTest::suite());
    test->add(DayCounterTest::suite());
    test->add(DefaultProbabilityCurveTest::suite());
    test->add(DigitalCouponTest::suite()); // might fail with QL_USE_INDEXED_COUPON
    test->add(DigitalOptionTest::suite());
    test->add(DistributionTest::suite());
    test->add(DividendOptionTest::suite());
    test->add(EuropeanOptionTest::suite());
    test->add(EverestOptionTest::suite());
    test->add(ExchangeRateTest::suite());
    test->add(ExtendedTreesTest::suite());
    test->add(FactorialTest::suite());
    test->add(FastFourierTransformTest::suite());
    test->add(FdHestonTest::suite());
    test->add(FdmLinearOpTest::suite());
    test->add(ForwardOptionTest::suite());
    test->add(GaussianQuadraturesTest::suite());
    test->add(GJRGARCHModelTest::suite());
    test->add(HestonModelTest::suite());
    test->add(HimalayaOptionTest::suite());
    test->add(HybridHestonHullWhiteProcessTest::suite());
    test->add(InflationTest::suite());                  // in general inflation & tests
    test->add(InflationCapFloorTest::suite());          // use indexed coupons, i.e.
    test->add(InflationCapFlooredCouponTest::suite());  // QL_USE_INDEXED_COUPON has no effect
    test->add(InflationVolTest::suite());               //
    test->add(InstrumentTest::suite());
    test->add(IntegralTest::suite());
    test->add(InterestRateTest::suite());
    test->add(InterpolationTest::suite());
    test->add(JumpDiffusionTest::suite());
    test->add(LinearLeastSquaresRegressionTest::suite());
    test->add(LookbackOptionTest::suite());
    test->add(LowDiscrepancyTest::suite());
    test->add(MargrabeOptionTest::suite());
    test->add(MarketModelTest::suite());
    test->add(MarketModelCmsTest::suite());
    test->add(MarketModelSmmTest::suite());
    test->add(MarketModelSmmCapletAlphaCalibrationTest::suite());
    test->add(MarketModelSmmCapletCalibrationTest::suite());
    test->add(MarketModelSmmCapletHomoCalibrationTest::suite());
    test->add(MatricesTest::suite());
    test->add(MCLongstaffSchwartzEngineTest::suite());
    test->add(MersenneTwisterTest::suite());
    test->add(MoneyTest::suite());
    test->add(NthToDefaultTest::suite());
    test->add(OperatorTest::suite());
    test->add(OptimizersTest::suite());
    test->add(OptionletStripperTest::suite());
    test->add(OvernightIndexedSwapTest::suite());
    test->add(PagodaOptionTest::suite());
    test->add(PathGeneratorTest::suite());
    test->add(PeriodTest::suite());
    test->add(PiecewiseYieldCurveTest::suite());
    test->add(QuantoOptionTest::suite());
    test->add(QuoteTest::suite());
    test->add(RiskStatisticsTest::suite());
    test->add(RngTraitsTest::suite());
    test->add(RoundingTest::suite());
    test->add(SampledCurveTest::suite());
    test->add(ShortRateModelTest::suite()); // fails with QL_USE_INDEXED_COUPON
    test->add(Solver1DTest::suite());
    test->add(StatisticsTest::suite());
    test->add(SurfaceTest::suite());
    test->add(SwapTest::suite());
    test->add(SwapForwardMappingsTest::suite());
    test->add(SwaptionTest::suite());
    test->add(SwaptionVolatilityCubeTest::suite());
    test->add(SwaptionVolatilityMatrixTest::suite());
    test->add(TermStructureTest::suite());
    test->add(TimeSeriesTest::suite());
    test->add(TqrEigenDecompositionTest::suite());
    test->add(TracingTest::suite());
    test->add(TransformedGridTest::suite());
    test->add(VarianceGammaTest::suite());
    test->add(VarianceOptionTest::suite());
    test->add(VarianceSwapTest::suite());
    test->add(VolatilityModelsTest::suite());

    //tests for deprecated classes
    test->add(LiborMarketModelTest::suite());
    test->add(LiborMarketModelProcessTest::suite());

    test->add(QUANTLIB_TEST_CASE(stopTimer));

    return test;
}
