/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017, 2018 Klaus Spanderen

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

#include "andreasenhugevolatilityinterpl.hpp"
#include "utilities.hpp"
#include <ql/math/comparison.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/math/optimization/bfgs.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugelocalvoladapter.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugevolatilityinterpl.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugevolatilityadapter.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace andreasen_huge_volatility_interpl_test {

    struct CalibrationData {
        const Handle<Quote> spot;
        Handle<YieldTermStructure> rTS, qTS;
        AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;
    };

    struct CalibrationResults {
        AndreasenHugeVolatilityInterpl::CalibrationType calibrationType;
        AndreasenHugeVolatilityInterpl::InterpolationType interpolationType;

        Real maxError, avgError;
        Real lvMaxError, lvAvgError;
    };

    CalibrationData AndreasenHugeExampleData() {
        // This is the example market data from the original paper
        // Andreasen J., Huge B., 2010. Volatility Interpolation
        // https://ssrn.com/abstract=1694972

        const Handle<Quote> spot(ext::make_shared<SimpleQuote>(2772.7));

        const Time maturityTimes[] = {
                  0.025, 0.101, 0.197, 0.274, 0.523, 0.772,
                  1.769, 2.267, 2.784, 3.781, 4.778, 5.774
            };

        const Real raw[][13] = {
            { 0.5131, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.3366, 0.3291, 0.0000, 0.0000 },
            { 0.5864, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.3178, 0.3129, 0.3008, 0.0000 },
            { 0.6597, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.3019, 0.2976, 0.2975, 0.0000 },
            { 0.7330, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.2863, 0.2848, 0.2848, 0.0000 },
            { 0.7697, 0.0000, 0.0000, 0.0000, 0.3262, 0.3079, 0.3001, 0.2843, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.8063, 0.0000, 0.0000, 0.0000, 0.3058, 0.2936, 0.2876, 0.2753, 0.2713, 0.2711, 0.2711, 0.2722, 0.2809 },
            { 0.8430, 0.0000, 0.0000, 0.0000, 0.2887, 0.2798, 0.2750, 0.2666, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.8613, 0.3365, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.8796, 0.3216, 0.2906, 0.2764, 0.2717, 0.2663, 0.2637, 0.2575, 0.2555, 0.2580, 0.2585, 0.2611, 0.2693 },
            { 0.8979, 0.3043, 0.2797, 0.2672, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.9163, 0.2880, 0.2690, 0.2578, 0.2557, 0.2531, 0.2519, 0.2497, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.9346, 0.2724, 0.2590, 0.2489, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.9529, 0.2586, 0.2488, 0.2405, 0.2407, 0.2404, 0.2411, 0.2418, 0.2410, 0.2448, 0.2469, 0.2501, 0.2584 },
            { 0.9712, 0.2466, 0.2390, 0.2329, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 0.9896, 0.2358, 0.2300, 0.2253, 0.2269, 0.2284, 0.2299, 0.2347, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.0079, 0.2247, 0.2213, 0.2184, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.0262, 0.2159, 0.2140, 0.2123, 0.2142, 0.2173, 0.2198, 0.2283, 0.2275, 0.2322, 0.2384, 0.2392, 0.2486 },
            { 1.0445, 0.2091, 0.2076, 0.2069, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.0629, 0.2056, 0.2024, 0.2025, 0.2039, 0.2074, 0.2104, 0.2213, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.0812, 0.2045, 0.1982, 0.1984, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.0995, 0.2025, 0.1959, 0.1944, 0.1962, 0.1988, 0.2022, 0.2151, 0.2161, 0.2219, 0.2269, 0.2305, 0.2399 },
            { 1.1178, 0.1933, 0.1929, 0.1920, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.1362, 0.0000, 0.0000, 0.0000, 0.1902, 0.1914, 0.1950, 0.2091, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.1728, 0.0000, 0.0000, 0.0000, 0.1885, 0.1854, 0.1888, 0.2039, 0.2058, 0.2122, 0.2186, 0.2223, 0.2321 },
            { 1.2095, 0.0000, 0.0000, 0.0000, 0.1867, 0.1811, 0.1839, 0.1990, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000 },
            { 1.2461, 0.0000, 0.0000, 0.0000, 0.1871, 0.1785, 0.1793, 0.1945, 0.0000, 0.2054, 0.2103, 0.2164, 0.2251 },
            { 1.3194, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.1988, 0.2054, 0.2105, 0.2190 },
            { 1.3927, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.1930, 0.2002, 0.2054, 0.2135 },
            { 1.4660, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.1849, 0.1964, 0.2012, 0.0000 }
        };

        const DayCounter dc = Actual365Fixed();
        const Date today = Date(1, March, 2010);

        const Handle<YieldTermStructure> rTS(flatRate(today, 0.0, dc));
        const Handle<YieldTermStructure> qTS(flatRate(today, 0.0, dc));

        const Size nStrikes = LENGTH(raw);
        const Size nMaturities = LENGTH(maturityTimes);

        QL_REQUIRE(nMaturities == LENGTH(raw[1])-1, "check raw data");

        AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;

        calibrationSet.reserve(std::count_if(
            &raw[0][0], &raw[nStrikes-1][nMaturities]+1,
            not_zero<Real>()) - nStrikes);

        for (const auto & i : raw) {
            const Real strike = spot->value()*i[0];

            for (Size j=1; j < LENGTH(i); ++j) {
                if (i[j] > QL_EPSILON) {
                    const Date maturity
                        = today + Period(Size(365*maturityTimes[j-1]), Days);

                    const Volatility impliedVol = i[j];

                    calibrationSet.push_back(std::make_pair(
                        ext::make_shared<VanillaOption>(
                            ext::make_shared<PlainVanillaPayoff>(
                                (strike < spot->value())? Option::Put
                                                        : Option::Call,
                                strike),
                            ext::make_shared<EuropeanExercise>(maturity)),
                        ext::make_shared<SimpleQuote>(impliedVol))
                    );
                }
            }
        }

        return { spot, rTS, qTS, calibrationSet };
    }

    void testAndreasenHugeVolatilityInterpolation(
        const CalibrationData& data, const CalibrationResults& expected) {

        SavedSettings backup;

        const Handle<YieldTermStructure> rTS = data.rTS;
        const Handle<YieldTermStructure> qTS = data.qTS;

        const DayCounter dc = rTS->dayCounter();
        const Date today = rTS->referenceDate();
        Settings::instance().evaluationDate() = today;

        const Handle<Quote> spot = data.spot;

        AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet =
            data.calibrationSet;

        const ext::shared_ptr<AndreasenHugeVolatilityInterpl>
            andreasenHugeVolInterplation(
                ext::make_shared<AndreasenHugeVolatilityInterpl>(
                    calibrationSet, spot, rTS, qTS,
                    expected.interpolationType,
                    expected.calibrationType));

        const ext::tuple<Real, Real, Real> error =
            andreasenHugeVolInterplation->calibrationError();

        const Real maxError = ext::get<1>(error);
        const Real avgError = ext::get<2>(error);

        if (maxError > expected.maxError || avgError > expected.avgError) {
            BOOST_FAIL("Failed to reproduce calibration error"
                       << "\n    max calibration error:     " << maxError
                       << "\n    average calibration error: " << avgError
                       << "\n    expected max error:        " << expected.maxError
                       << "\n    expected average error:    " << expected.avgError);
        }

        const ext::shared_ptr<AndreasenHugeVolatilityAdapter> volatilityAdapter(
            ext::make_shared<AndreasenHugeVolatilityAdapter>(
                andreasenHugeVolInterplation, 1e-12));

        const ext::shared_ptr<AndreasenHugeLocalVolAdapter> localVolAdapter(
            ext::make_shared<AndreasenHugeLocalVolAdapter>(
                andreasenHugeVolInterplation));

        const ext::shared_ptr<GeneralizedBlackScholesProcess> localVolProcess(
            ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS,
                Handle<BlackVolTermStructure>(volatilityAdapter),
                Handle<LocalVolTermStructure>(localVolAdapter)));

        Real lvAvgError = 0.0, lvMaxError = 0.0;
        for (Size i=0, n=0; i < calibrationSet.size(); ++i) {

            const ext::shared_ptr<VanillaOption> option =
                calibrationSet[i].first;

            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                    option->payoff());
            const Real strike = payoff->strike();
            const Option::Type optionType = payoff->optionType();

            const Time t = dc.yearFraction(today, option->exercise()->lastDate());

            const Volatility expectedVol = calibrationSet[i].second->value();
            const Volatility calculatedVol =
                volatilityAdapter->blackVol(t, strike, true);

            const Real diffVol = std::fabs(expectedVol - calculatedVol);
            const Real tol = std::max(1e-10, 1.01*maxError);

            if (diffVol > tol) {
                BOOST_FAIL("Failed to reproduce calibration option price"
                           << "\n    calculated: " << calculatedVol
                           << "\n    expected:   " << expectedVol
                           << "\n    difference: " << diffVol
                           << "\n    tolerance:  " << tol);
            }

            const ext::shared_ptr<PricingEngine> fdEngine(
                ext::make_shared<FdBlackScholesVanillaEngine>(
                    localVolProcess, std::max<Size>(30, Size(100*t)),
                    200, 0, FdmSchemeDesc::Douglas(), true));

            option->setPricingEngine(fdEngine);

            const DiscountFactor discount = rTS->discount(t);
            const Real fwd = spot->value()*qTS->discount(t)/discount;

            const Volatility lvImpliedVol = blackFormulaImpliedStdDevLiRS(
                optionType, strike, fwd, option->NPV(),
                discount, 0.0, Null<Real>(), 1.0, 1e-12)/std::sqrt(t);

            const Real lvError = std::fabs(lvImpliedVol - expectedVol);

            lvMaxError = std::max(lvError, lvMaxError);

            lvAvgError = (n*lvAvgError + lvError)/(n+1);

            ++n;
        }

        if (lvMaxError > expected.lvMaxError || avgError > expected.lvAvgError) {
            BOOST_FAIL("Failed to reproduce local volatility calibration error"
                       << "\n    max calibration error:     " << lvMaxError
                       << "\n    average calibration error: " << lvAvgError
                       << "\n    expected max error:        " << expected.lvMaxError
                       << "\n    expected average error:    " << expected.lvAvgError);
        }
    }


    CalibrationData BorovkovaExampleData() {
        // see Svetlana Borovkova, Ferry J. Permana
        // Implied volatility in oil markets
        // http://www.researchgate.net/publication/46493859_Implied_volatility_in_oil_markets

        const DayCounter dc = Actual365Fixed();
        const Date today = Date(4, January, 2018);

        const Handle<YieldTermStructure> rTS(flatRate(today, 0.025, dc));
        const Handle<YieldTermStructure> qTS(flatRate(today, 0.085, dc));

        Handle<Quote> spot(ext::make_shared<SimpleQuote>(100));

        const Real b1 = 0.35;
        const Real b2 = 0.03;
        const Real b3 = 0.005;
        const Real b4 = -0.02;
        const Real b5 = -0.005;

        const Real strikes[] = { 35, 50, 75, 100, 125, 150, 200, 300 };
        const Size maturityMonths[] = { 1, 3, 6, 9, 12, 15, 18, 24};

        AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;

        for (double strike : strikes) {
            for (unsigned long maturityMonth : maturityMonths) {
                const Date maturityDate = today + Period(maturityMonth, Months);
                const Time t = dc.yearFraction(today, maturityDate);

                const Real fwd = spot->value()*qTS->discount(t)/rTS->discount(t);
                const Real mn = std::log(fwd/strike)/std::sqrt(t);

                const Volatility vol = b1 + b2*mn + b3*mn*mn + b4*t + b5*mn*t;

                if (std::fabs(mn) < 3.71*vol) {

                    calibrationSet.push_back(std::make_pair(
                        ext::make_shared<VanillaOption>(
                            ext::make_shared<PlainVanillaPayoff>(
                                Option::Call, strike),
                            ext::make_shared<EuropeanExercise>(maturityDate)),
                        ext::make_shared<SimpleQuote>(vol)));
                }
            }
        }

        CalibrationData data = { spot, rTS, qTS, calibrationSet };

        return data;
    }


    CalibrationData arbitrageData() {

        const DayCounter dc = Actual365Fixed();
        const Date today = Date(4, January, 2018);

        const Handle<YieldTermStructure> rTS(flatRate(today, 0.13, dc));
        const Handle<YieldTermStructure> qTS(flatRate(today, 0.03, dc));

        Handle<Quote> spot(ext::make_shared<SimpleQuote>(100));

        const Real strikes[] = { 100, 100, 100, 150 };
        const Size maturities[] = { 1, 3, 6, 6 };
        const Volatility vols[] = { 0.25, 0.35, 0.05, 0.35 };
        AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;

        for (Size i=0; i < LENGTH(strikes); ++i) {
            const Real strike = strikes[i];
            const Date maturityDate = today + Period(maturities[i], Months);
            const Volatility vol = vols[i];

            calibrationSet.push_back(std::make_pair(
                ext::make_shared<VanillaOption>(
                    ext::make_shared<PlainVanillaPayoff>(
                        Option::Call, strike),
                    ext::make_shared<EuropeanExercise>(maturityDate)),
                ext::make_shared<SimpleQuote>(vol)));
        }

        return { spot, rTS, qTS, calibrationSet };
    }
}


void AndreasenHugeVolatilityInterplTest::testAndreasenHugePut() {

    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge example with Put calibration...");

    using namespace andreasen_huge_volatility_interpl_test;

    const CalibrationData data = AndreasenHugeExampleData();

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::Put,
        AndreasenHugeVolatilityInterpl::CubicSpline,
        0.0015, 0.00035,
        0.0020, 0.00035
    };

    testAndreasenHugeVolatilityInterpolation(data, expected);
}

void AndreasenHugeVolatilityInterplTest::testAndreasenHugeCall() {

    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge example with Call calibration...");

    using namespace andreasen_huge_volatility_interpl_test;

    const CalibrationData data = AndreasenHugeExampleData();

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::Call,
        AndreasenHugeVolatilityInterpl::CubicSpline,
        0.0015, 0.00035,
        0.0015, 0.00035
    };

    testAndreasenHugeVolatilityInterpolation(data, expected);
}

void AndreasenHugeVolatilityInterplTest::testAndreasenHugeCallPut() {

    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge example with instantaneous "
         "Call and Put calibration...");

    using namespace andreasen_huge_volatility_interpl_test;

    const CalibrationData data = AndreasenHugeExampleData();

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::CallPut,
        AndreasenHugeVolatilityInterpl::CubicSpline,
        0.0015, 0.00035,
        0.0015, 0.00035
    };

    testAndreasenHugeVolatilityInterpolation(data, expected);
}

void AndreasenHugeVolatilityInterplTest::testLinearInterpolation() {
    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge example with linear interpolation...");

    using namespace andreasen_huge_volatility_interpl_test;

    const CalibrationData data = AndreasenHugeExampleData();

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::CallPut,
        AndreasenHugeVolatilityInterpl::Linear,
        0.0020, 0.00015,
        0.0040, 0.00035
    };

    testAndreasenHugeVolatilityInterpolation(data, expected);
}

void AndreasenHugeVolatilityInterplTest::testPiecewiseConstantInterpolation() {
    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge example with piecewise constant interpolation...");

    using namespace andreasen_huge_volatility_interpl_test;

    const CalibrationData data = AndreasenHugeExampleData();

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::CallPut,
        AndreasenHugeVolatilityInterpl::PiecewiseConstant,
        0.0025, 0.00025,
        0.0040, 0.00035
    };

    testAndreasenHugeVolatilityInterpolation(data, expected);
}

void AndreasenHugeVolatilityInterplTest::testTimeDependentInterestRates() {

    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge volatility interpolation with "
        "time dependent interest rates and dividend yield...");

    using namespace andreasen_huge_volatility_interpl_test;

    SavedSettings backup;

    const CalibrationData data = AndreasenHugeExampleData();

    const DayCounter dc = data.rTS->dayCounter();
    const Date today = data.rTS->referenceDate();
    Settings::instance().evaluationDate() = today;

    std::vector<Real> r = { 0.0167, 0.023, 0.03234, 0.034, 0.038, 0.042, 0.047, 0.053 };
    std::vector<Real> q = { 0.01, 0.011, 0.013, 0.014, 0.02, 0.025, 0.067, 0.072 };

    std::vector<Date> dates = {
        today,
        today + Period(41, Days),
        today + Period(75, Days),
        today + Period(165, Days),
        today + Period(256, Days),
        today + Period(345, Days),
        today + Period(524, Days),
        today + Period(2190, Days)
    };

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<ZeroCurve>(dates, r, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<ZeroCurve>(dates, q, dc));

    const CalibrationData origData = AndreasenHugeExampleData();
    AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet =
        origData.calibrationSet;

    const Handle<Quote> spot = origData.spot;

    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, spot, 0.09, 2.0, 0.09, 0.4, -0.75)));

    const ext::shared_ptr<PricingEngine> hestonEngine(
        ext::make_shared<AnalyticHestonEngine>(
            hestonModel,
            AnalyticHestonEngine::AndersenPiterbarg,
            AnalyticHestonEngine::Integration::discreteTrapezoid(128)));

    for (auto& i : calibrationSet) {
        const ext::shared_ptr<VanillaOption> option = i.first;

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(option->payoff());

        const Real strike = payoff->strike();
        const Option::Type optionType = payoff->optionType();

        const Time t = dc.yearFraction(today, option->exercise()->lastDate());

        const DiscountFactor discount = rTS->discount(t);
        const Real fwd = spot->value()*qTS->discount(t)/discount;

        option->setPricingEngine(hestonEngine);
        const Real npv = option->NPV();

        const Volatility impliedVol = blackFormulaImpliedStdDevLiRS(
            optionType, strike, fwd, npv,
            discount, 0.0, Null<Real>(), 1.0, 1e-12)/std::sqrt(t);

        i.second = ext::make_shared<SimpleQuote>(impliedVol);
    }

    CalibrationData irData = { spot, rTS, qTS, calibrationSet };

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::CallPut,
        AndreasenHugeVolatilityInterpl::CubicSpline,
        0.0020, 0.0003,
        0.0020, 0.0004
    };

    testAndreasenHugeVolatilityInterpolation(irData, expected);
}

void AndreasenHugeVolatilityInterplTest::testSingleOptionCalibration() {
    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge volatility interpolation with "
        "a single option...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(4, January, 2018);

    const Handle<YieldTermStructure> rTS(flatRate(today, 0.025, dc));
    const Handle<YieldTermStructure> qTS(flatRate(today, 0.085, dc));

    AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;

    const Real strike = 10.0;
    const Volatility vol = 0.3;
    const Date maturity = today + Period(1, Years);
    Handle<Quote> spot(ext::make_shared<SimpleQuote>(strike));

    calibrationSet.push_back(std::make_pair(
        ext::make_shared<VanillaOption>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
            ext::make_shared<EuropeanExercise>(maturity)),
        ext::make_shared<SimpleQuote>(vol)));

    const AndreasenHugeVolatilityInterpl::InterpolationType interpl[] = {
        AndreasenHugeVolatilityInterpl::Linear,
        AndreasenHugeVolatilityInterpl::CubicSpline,
        AndreasenHugeVolatilityInterpl::PiecewiseConstant
    };

    const AndreasenHugeVolatilityInterpl::CalibrationType calibrationType[] = {
        AndreasenHugeVolatilityInterpl::Put,
        AndreasenHugeVolatilityInterpl::Call,
        AndreasenHugeVolatilityInterpl::CallPut
    };

    for (auto i : interpl)
        for (auto j : calibrationType) {
            const ext::shared_ptr<AndreasenHugeVolatilityInterpl> andreasenHugeVolInterplation(
                ext::make_shared<AndreasenHugeVolatilityInterpl>(calibrationSet, spot, rTS, qTS, i,
                                                                 j, 50));

            const ext::shared_ptr<AndreasenHugeVolatilityAdapter>
                volatilityAdapter =
                    ext::make_shared<AndreasenHugeVolatilityAdapter>(
                        andreasenHugeVolInterplation);

            const Volatility calculated =
                volatilityAdapter->blackVol(maturity, strike);
            const Volatility expected = vol;

            if (std::fabs(calculated - expected) > 1e-4) {
                BOOST_FAIL("Failed to reproduce single option calibration"
                           << "\n    calculated: " << calculated
                           << "\n    expected:   " << expected);
            }
        }
}

void AndreasenHugeVolatilityInterplTest::testArbitrageFree() {
    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge volatility interpolation gives "
        "arbitrage free prices...");

    using namespace andreasen_huge_volatility_interpl_test;

    SavedSettings backup;

    CalibrationData data[] = { BorovkovaExampleData(), arbitrageData() };;

    for (auto& i : data) {
        const Handle<Quote>& spot = i.spot;
        const AndreasenHugeVolatilityInterpl::CalibrationSet& calibrationSet = i.calibrationSet;

        const Handle<YieldTermStructure>& rTS = i.rTS;
        const Handle<YieldTermStructure>& qTS = i.qTS;

        const DayCounter dc = rTS->dayCounter();
        const Date today = rTS->referenceDate();

        const ext::shared_ptr<AndreasenHugeVolatilityInterpl>
            andreasenHugeVolInterplation(
                ext::make_shared<AndreasenHugeVolatilityInterpl>(
                    calibrationSet, spot, rTS, qTS,
                    AndreasenHugeVolatilityInterpl::CubicSpline,
                    AndreasenHugeVolatilityInterpl::CallPut, 5000));

        const ext::shared_ptr<AndreasenHugeVolatilityAdapter> volatilityAdapter(
            ext::make_shared<AndreasenHugeVolatilityAdapter>(
                andreasenHugeVolInterplation));

        for (Real m = -0.7; m < 0.7; m+=0.05) {

            for (Size weeks=6; weeks < 52; ++weeks) {
                const Date maturityDate = today + Period(weeks, Weeks);

                const Time t = dc.yearFraction(today, maturityDate);

                const Real fwd = spot->value()*qTS->discount(t)/rTS->discount(t);

                // J. Gatheral, Arbitrage-free SVI volatility surfaces
                // http://mfe.baruch.cuny.edu/wp-content/uploads/2013/01/OsakaSVI2012.pdf
                const Real eps = 0.025;
                const Real k  = fwd*std::exp(m);
                const Real km = fwd*std::exp(m - eps);
                const Real kp = fwd*std::exp(m + eps);

                const Real w =
                    volatilityAdapter->blackVariance(t, k, true);
                const Real w_p =
                    volatilityAdapter->blackVariance(t, kp, true);
                const Real w_m =
                    volatilityAdapter->blackVariance(t, km, true);

                const Real w1 = (w_p - w_m)/(2*eps);
                const Real w2 = (w_p + w_m - 2*w)/(eps*eps);

                const Real g_k = square<Real>()(1-m*w1/(2*w))
                    - w1*w1/4*(1/w + 0.25) + 0.5*w2;

                if (g_k < 0) {
                    BOOST_FAIL("No-arbitrage condition g_k >= 0 failed"
                               << "\n    strike:  " << k
                               << "\n    forward: " << fwd
                               << "\n    time:    " << t
                               << "\n    g_k:    " << g_k);
                }

                const Real deltaT = 1.0/365.;
                const Real fwdpt = spot->value()*
                    qTS->discount(t+deltaT)/rTS->discount(t+deltaT);

                const Real kpt = fwdpt*std::exp(m);
                const Real w_pt =
                    volatilityAdapter->blackVariance(t+deltaT, kpt, true);

                const Real w_t = (w_pt - w)/deltaT;
                if (w_t < -1e-8) {
                    BOOST_FAIL("No-arbitrage condition w_t >= 0 failed"
                               << "\n    strike:  " << k
                               << "\n    forward: " << fwd
                               << "\n    time:    " << t
                               << "\n    w        " << w
                               << "\n    w_t:     " << w_t);
                }
            }
        }
    }
}

void AndreasenHugeVolatilityInterplTest::testBarrierOptionPricing() {
    BOOST_TEST_MESSAGE(
        "Testing Barrier option pricing with Andreasen-Huge "
         "local volatility surface...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(4, January, 2018);

    const Handle<YieldTermStructure> rTS(flatRate(today, 0.01, dc));
    const Handle<YieldTermStructure> qTS(flatRate(today, 0.03, dc));

    Handle<Quote> spot(ext::make_shared<SimpleQuote>(100));
    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, spot, 0.04, 2.0, 0.04, 0.4, -0.75)));

    const ext::shared_ptr<BlackVolTermStructure> hestonVol =
        ext::make_shared<HestonBlackVolSurface>(
            Handle<HestonModel>(hestonModel));

    const ext::shared_ptr<GeneralizedBlackScholesProcess>
        dupireLocalVolProcess =
            ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS, Handle<BlackVolTermStructure>(hestonVol));

    const Real strikes[] = { 25, 50, 75, 90, 100, 110, 125, 150, 200, 400};
    const Size maturityMonths[] = { 1, 3, 6, 9, 12};

    AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;

    for (double strike : strikes) {
        for (unsigned long maturityMonth : maturityMonths) {
            const Date maturityDate = today + Period(maturityMonth, Months);
            const Time t = dc.yearFraction(today, maturityDate);

            const Volatility vol = hestonVol->blackVol(t, strike);

            const Real mn = std::log(spot->value()/strike)/std::sqrt(t);

            if (std::fabs(mn) < 3.07*vol) {
                calibrationSet.push_back(std::make_pair(
                    ext::make_shared<VanillaOption>(
                        ext::make_shared<PlainVanillaPayoff>(
                            Option::Call, strike),
                        ext::make_shared<EuropeanExercise>(maturityDate)),
                    ext::make_shared<SimpleQuote>(vol)));
            }
        }
    }

    const ext::shared_ptr<AndreasenHugeVolatilityInterpl>
        andreasenHugeVolInterplation(
            ext::make_shared<AndreasenHugeVolatilityInterpl>(
                calibrationSet, spot, rTS, qTS));

    const ext::shared_ptr<AndreasenHugeLocalVolAdapter> localVolAdapter(
        ext::make_shared<AndreasenHugeLocalVolAdapter>(
            andreasenHugeVolInterplation));

    const ext::shared_ptr<GeneralizedBlackScholesProcess>
        andreasenHugeLocalVolProcess =
            ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS,
                Handle<BlackVolTermStructure>(hestonVol),
                Handle<LocalVolTermStructure>(localVolAdapter));

    const Real strike = 120.0;
    const Real barrier=  80.0;
    const Real rebate =   0.0;
    const Date maturity = today + Period(1, Years);
    const Barrier::Type barrierType = Barrier::DownOut;

    BarrierOption barrierOption(barrierType, barrier, rebate,
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike),
        ext::make_shared<EuropeanExercise>(maturity));

    barrierOption.setPricingEngine(
        ext::make_shared<FdBlackScholesBarrierEngine>(
            dupireLocalVolProcess, 50, 100, 0,
            FdmSchemeDesc::Douglas(), true, 0.2));

    const Real dupireNPV = barrierOption.NPV();

    barrierOption.setPricingEngine(
        ext::make_shared<FdBlackScholesBarrierEngine>(
            andreasenHugeLocalVolProcess, 200, 400, 0,
            FdmSchemeDesc::Douglas(), true, 0.25));

    const Real andreasenHugeNPV = barrierOption.NPV();

    const Real tol = 0.15;
    const Real diff = std::fabs(andreasenHugeNPV - dupireNPV);

    if (diff > tol) {
        BOOST_FAIL("failed to reproduce barrier prices with Andreasen-Huge "
                "local volatility surface"
                   << "\n    Andreasen-Huge price: " << andreasenHugeNPV
                   << "\n    Dupire formula price: " << dupireNPV
                   << "\n    diff:                 " << diff
                   << "\n    tolerance:            " << tol);
    }
}

namespace andreasen_huge_volatility_interpl_test {
    std::pair<CalibrationData, std::vector<Real> > sabrData() {

        const DayCounter dc = Actual365Fixed();
        const Date today = Date(4, January, 2018);

        const Real alpha = 0.15;
        const Real beta = 0.8;
        const Real nu = 0.5;
        const Real rho = -0.48;
        const Real forward = 0.03;
        const Size maturityInYears = 20;

        const Date maturityDate = today + Period(maturityInYears, Years);
        const Time maturity = dc.yearFraction(today, maturityDate);

        AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;

        const Real strikes[] = { 0.02, 0.025, 0.03, 0.035, 0.04, 0.05, 0.06 };

        for (double strike : strikes) {
            const Volatility vol = sabrVolatility(strike, forward, maturity, alpha, beta, nu, rho);

            calibrationSet.push_back(std::make_pair(
                ext::make_shared<VanillaOption>(
                    ext::make_shared<PlainVanillaPayoff>(
                        Option::Call, strike),
                    ext::make_shared<EuropeanExercise>(maturityDate)),
                ext::make_shared<SimpleQuote>(vol)));
        }

        const Handle<YieldTermStructure> rTS(flatRate(today, forward, dc));
        const Handle<YieldTermStructure> qTS(flatRate(today, forward, dc));

        Handle<Quote> spot(ext::make_shared<SimpleQuote>(forward));

        const CalibrationData data = { spot, rTS, qTS, calibrationSet};

        std::vector<Real> parameter = { alpha, beta, nu, rho, forward, maturity };

        return std::make_pair(data, parameter);
    }
}

void AndreasenHugeVolatilityInterplTest::testPeterAndFabiensExample() {
    BOOST_TEST_MESSAGE(
        "Testing Peter's and Fabien's SABR example...");

    // http://chasethedevil.github.io/post/andreasen-huge-extrapolation/

    using namespace andreasen_huge_volatility_interpl_test;

    SavedSettings backup;

    const std::pair<CalibrationData, std::vector<Real> > sd = sabrData();
    const CalibrationData& data = sd.first;
    const std::vector<Real>& parameter = sd.second;

    const ext::shared_ptr<AndreasenHugeVolatilityInterpl>
        andreasenHugeVolInterplation(
            ext::make_shared<AndreasenHugeVolatilityInterpl>(
                data.calibrationSet, data.spot, data.rTS, data.qTS));

    const ext::shared_ptr<AndreasenHugeVolatilityAdapter> volAdapter(
        ext::make_shared<AndreasenHugeVolatilityAdapter>(
            andreasenHugeVolInterplation));

    const Real alpha    = parameter[0];
    const Real beta     = parameter[1];
    const Real nu       = parameter[2];
    const Real rho      = parameter[3];
    const Real forward  = parameter[4];
    const Time maturity = parameter[5];

    for (Real strike = 0.02; strike < 0.06; strike+=0.001) {
        const Volatility sabrVol = sabrVolatility(
           strike, forward, maturity, alpha, beta, nu, rho);

        const Volatility ahVol = volAdapter->blackVol(maturity, strike, true);

        const Real tol = 0.0005;
        const Real diff = std::fabs(sabrVol - ahVol);

        if (std::isnan(ahVol) || diff > 0.005) {
            BOOST_FAIL("failed to reproduce SABR volatility with "
                    "Andreasen-Huge interpolation"
                   << "\n    Andreasen-Huge vol: " << ahVol
                   << "\n    SABR volatility:    " << sabrVol
                   << "\n    diff:               " << diff
                   << "\n    tolerance:          " << tol);
        }
    }
}

void AndreasenHugeVolatilityInterplTest::testDifferentOptimizers() {
    BOOST_TEST_MESSAGE(
        "Testing different optimizer for Andreasen-Huge "
        "volatility interpolation...");

    using namespace andreasen_huge_volatility_interpl_test;

    const CalibrationData& data = sabrData().first;

    const ext::shared_ptr<OptimizationMethod> optimizationMethods[] = {
        ext::make_shared<LevenbergMarquardt>(),
        ext::make_shared<BFGS>(),
        ext::make_shared<Simplex>(0.2)
    };

    for (const auto& optimizationMethod : optimizationMethods) {
        const Real avgError = ext::get<2>(
            AndreasenHugeVolatilityInterpl(data.calibrationSet, data.spot, data.rTS, data.qTS,
                                           AndreasenHugeVolatilityInterpl::CubicSpline,
                                           AndreasenHugeVolatilityInterpl::Call, 400, Null<Real>(),
                                           Null<Real>(), optimizationMethod)
                .calibrationError());

        if (std::isnan(avgError) || avgError > 0.0001) {
            BOOST_FAIL("failed to calibrate Andreasen-Huge "
                    "volatility interpolation with different optimizera"
                   << "\n    calibration error: " << avgError);
        }
    }
}

void AndreasenHugeVolatilityInterplTest::testMovingReferenceDate() {
    BOOST_TEST_MESSAGE(
        "Testing that reference date of adapter surface moves along with "
        "evaluation date...");

    SavedSettings backup;

    const Date today = Date(4, January, 2018);
    Settings::instance().evaluationDate() = today;

    const DayCounter dc = Actual365Fixed();
    const Date maturity = today + Period(1, Months);

    Handle<YieldTermStructure> ts(flatRate(0.04, dc));

    const Real s0 = 100.0;
    const Volatility impliedVol = 0.2;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));

    AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet(
        1,
        std::make_pair(
            ext::make_shared<VanillaOption>(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, s0),
                ext::make_shared<EuropeanExercise>(maturity)),
            ext::make_shared<SimpleQuote>(impliedVol))
    );

    const ext::shared_ptr<AndreasenHugeVolatilityInterpl>
        andreasenHugeVolInterplation(
            ext::make_shared<AndreasenHugeVolatilityInterpl>(
                calibrationSet, spot, ts, ts));


    const Real tol = 1e-8;
    const ext::shared_ptr<AndreasenHugeVolatilityAdapter> volatilityAdapter(
        ext::make_shared<AndreasenHugeVolatilityAdapter>(
            andreasenHugeVolInterplation, tol));

    const ext::shared_ptr<AndreasenHugeLocalVolAdapter> localVolAdapter(
        ext::make_shared<AndreasenHugeLocalVolAdapter>(
            andreasenHugeVolInterplation));

    const Date volRefDate = volatilityAdapter->referenceDate();
    const Date localRefDate = localVolAdapter->referenceDate();

    if (volRefDate != today || localRefDate != today)
        BOOST_FAIL("reference dates should match today's date"
               << "\n    today                     : " << today
               << "\n    local vol reference date  : " << localRefDate
               << "\n    implied vol reference date: " << volRefDate);

    const Date modToday = Date(15, January, 2018);
    Settings::instance().evaluationDate() = modToday;

    const Date modVolRefDate = volatilityAdapter->referenceDate();
    const Date modLocalRefDate = localVolAdapter->referenceDate();

    if (modVolRefDate != modToday || modLocalRefDate != modToday)
        BOOST_FAIL("reference dates should match modified today's date"
               << "\n    today                     : " << modToday
               << "\n    local vol reference date  : " << modLocalRefDate
               << "\n    implied vol reference date: " << modVolRefDate);

    // test update method
    const Volatility modImpliedVol =
        volatilityAdapter->blackVol(maturity, s0, true);

    const Real diff = std::fabs(modImpliedVol - impliedVol);
    if (diff > 10*tol)
        BOOST_FAIL("modified implied vol should match direct calculation"
                << "\n    implied vol         : " << impliedVol
                << "\n    modified implied vol: " << modImpliedVol
                << "\n    difference          : " << diff
                << "\n    tolerance           : " << tol);
}

void AndreasenHugeVolatilityInterplTest::testFlatVolCalibration() {
    BOOST_TEST_MESSAGE(
        "Testing Andreasen-Huge example with flat volatility surface...");

    using namespace andreasen_huge_volatility_interpl_test;

    SavedSettings backup;

    const Date ref(1, November, 2019);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = ref;

    const Date expiries[] = {
        ref + 1 * Months, ref + 3 * Months, ref + 6 * Months,
        ref + 9 * Months, ref + 1 * Years,  ref + 2 * Years,
        ref + 3 * Years,  ref + 4 * Years,  ref + 5 * Years,
        ref + 7 * Years,  ref + 10 * Years
    };

    const Real moneyness[] = {
        0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5
    };

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));
    const Handle<YieldTermStructure> rTS(flatRate(ref, 0.02, dc));
    const Handle<YieldTermStructure> qTS(flatRate(ref, 0.0, dc));
    const ext::shared_ptr<Quote> vol = ext::make_shared<SimpleQuote>(0.18);

    AndreasenHugeVolatilityInterpl::CalibrationSet calibrationSet;
    for (auto expiry : expiries) {

        const ext::shared_ptr<Exercise> exercise = ext::make_shared<EuropeanExercise>(expiry);

        const Time t = rTS->timeFromReference(expiry);
        const Real fwd = spot->value() / rTS->discount(t) * qTS->discount(t);

        for (double m : moneyness) {
            const Real strike = fwd * m;
            const Real mn = std::log(fwd/strike)/std::sqrt(t);

            if (std::fabs(mn) < 3.72*vol->value()) {
                const ext::shared_ptr<VanillaOption> option
                    = ext::make_shared<VanillaOption>(
                          ext::make_shared<PlainVanillaPayoff>(
                              (strike>fwd)? Option::Call : Option::Put, strike),
                          exercise);

                calibrationSet.push_back(std::make_pair(option, vol));
            }
        }
    }

    CalibrationData flatVolData = { spot, rTS, qTS, calibrationSet };

    const CalibrationResults expected = {
        AndreasenHugeVolatilityInterpl::Put,
        AndreasenHugeVolatilityInterpl::CubicSpline,
        1e-10, 1e-10,
        0.0006, 0.0002
    };

    testAndreasenHugeVolatilityInterpolation(flatVolData, expected);
}


test_suite* AndreasenHugeVolatilityInterplTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("Andreasen-Huge volatility interpolation tests");

    suite->add(QUANTLIB_TEST_CASE(
        &AndreasenHugeVolatilityInterplTest::testSingleOptionCalibration));
    suite->add(QUANTLIB_TEST_CASE(
        &AndreasenHugeVolatilityInterplTest::testArbitrageFree));
    suite->add(QUANTLIB_TEST_CASE(
        &AndreasenHugeVolatilityInterplTest::testPeterAndFabiensExample));
    suite->add(QUANTLIB_TEST_CASE(
        &AndreasenHugeVolatilityInterplTest::testDifferentOptimizers));
    suite->add(QUANTLIB_TEST_CASE(
        &AndreasenHugeVolatilityInterplTest::testMovingReferenceDate));
    suite->add(QUANTLIB_TEST_CASE(
        &AndreasenHugeVolatilityInterplTest::testFlatVolCalibration));

    if (speed == Slow) {
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testAndreasenHugePut));
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testAndreasenHugeCall));
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testAndreasenHugeCallPut));
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testLinearInterpolation));
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testPiecewiseConstantInterpolation));
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testBarrierOptionPricing));
        suite->add(QUANTLIB_TEST_CASE(
            &AndreasenHugeVolatilityInterplTest::testTimeDependentInterestRates));
    }
    return suite;
}

