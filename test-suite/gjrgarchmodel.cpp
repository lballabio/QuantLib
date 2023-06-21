/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Yee Man Chan

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

#include "gjrgarchmodel.hpp"
#include "utilities.hpp"
#include <ql/processes/gjrgarchprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/models/equity/gjrgarchmodel.hpp>
#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/pricingengines/vanilla/analyticgjrgarchengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeangjrgarchengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/time/period.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void GJRGARCHModelTest::testEngines() {
    BOOST_TEST_MESSAGE(
       "Testing Monte Carlo GJR-GARCH engine against "
       "analytic GJR-GARCH engine...");

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);

    const Date today = Date::todaysDate();
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, 0.05, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(today, 0.0, dayCounter));

    const Real s0 = 50.0;
    const Real omega = 2.0e-6;
    const Real alpha = 0.024;
    const Real beta = 0.93;
    const Real gamma = 0.059;
    const Real daysPerYear = 365.0; // number of trading days per year
    const Size maturity[] = {90, 180};
    const Real strike[] = {35,40,45,50,55,60};
    const Real Lambda[] = {0.0,0.1,0.2};
    Real analytic[3][2][6]; // correct values of analytic approximation
    analytic[0][0][0] = 15.4315;
    analytic[0][0][1] = 10.5552;
    analytic[0][0][2] = 5.9625;
    analytic[0][0][3] = 2.3282;
    analytic[0][0][4] = 0.5408;
    analytic[0][0][5] = 0.0835;
    analytic[0][1][0] = 15.8969;
    analytic[0][1][1] = 11.2173;
    analytic[0][1][2] = 6.9112;
    analytic[0][1][3] = 3.4788;
    analytic[0][1][4] = 1.3769;
    analytic[0][1][5] = 0.4357;
    analytic[1][0][0] = 15.4556;
    analytic[1][0][1] = 10.6929;
    analytic[1][0][2] = 6.2381;
    analytic[1][0][3] = 2.6831;
    analytic[1][0][4] = 0.7822;
    analytic[1][0][5] = 0.1738;
    analytic[1][1][0] = 16.0587;
    analytic[1][1][1] = 11.5338;
    analytic[1][1][2] = 7.3170;
    analytic[1][1][3] = 3.9074;
    analytic[1][1][4] = 1.7279;
    analytic[1][1][5] = 0.6568;
    analytic[2][0][0] = 15.8000;
    analytic[2][0][1] = 11.2734;
    analytic[2][0][2] = 7.0376;
    analytic[2][0][3] = 3.6767;
    analytic[2][0][4] = 1.5871;
    analytic[2][0][5] = 0.5934;
    analytic[2][1][0] = 16.9286;
    analytic[2][1][1] = 12.3170;
    analytic[2][1][2] = 8.0405;
    analytic[2][1][3] = 4.6348;
    analytic[2][1][4] = 2.3429;
    analytic[2][1][5] = 1.0590;
    Real mcValues[3][2][6]; // correct values of Monte Carlo
    mcValues[0][0][0] = 15.4332;
    mcValues[0][0][1] = 10.5453;
    mcValues[0][0][2] = 5.9351;
    mcValues[0][0][3] = 2.3521;
    mcValues[0][0][4] = 0.5597;
    mcValues[0][0][5] = 0.0776;
    mcValues[0][1][0] = 15.8910;
    mcValues[0][1][1] = 11.1772;
    mcValues[0][1][2] = 6.8827;
    mcValues[0][1][3] = 3.5096;
    mcValues[0][1][4] = 1.4196;
    mcValues[0][1][5] = 0.4502;
    mcValues[1][0][0] = 15.4580;
    mcValues[1][0][1] = 10.6433;
    mcValues[1][0][2] = 6.2019;
    mcValues[1][0][3] = 2.7513;
    mcValues[1][0][4] = 0.8374;
    mcValues[1][0][5] = 0.1706;
    mcValues[1][1][0] = 15.9884;
    mcValues[1][1][1] = 11.4139;
    mcValues[1][1][2] = 7.3103;
    mcValues[1][1][3] = 4.0497;
    mcValues[1][1][4] = 1.8862;
    mcValues[1][1][5] = 0.7322;
    mcValues[2][0][0] = 15.6619;
    mcValues[2][0][1] = 11.1263;
    mcValues[2][0][2] = 7.0968;
    mcValues[2][0][3] = 3.9152;
    mcValues[2][0][4] = 1.8133;
    mcValues[2][0][5] = 0.7010;
    mcValues[2][1][0] = 16.5195;
    mcValues[2][1][1] = 12.3181;
    mcValues[2][1][2] = 8.6085;
    mcValues[2][1][3] = 5.5700;
    mcValues[2][1][4] = 3.3103;
    mcValues[2][1][5] = 1.8053;

    for (Size k = 0; k < 3; ++k) {
        Real lambda = Lambda[k];
        Real m1 = beta+(alpha+gamma*CumulativeNormalDistribution()(lambda))
            *(1.0+lambda*lambda)+gamma*lambda*std::exp(-lambda*lambda/2.0)
            /std::sqrt(2.0*M_PI);
        Real v0 = omega/(1.0-m1);
        Handle<Quote> q(ext::shared_ptr<Quote>(new SimpleQuote(s0)));
        ext::shared_ptr<GJRGARCHProcess> process(new GJRGARCHProcess(
            riskFreeTS, dividendTS, q, v0, omega, alpha, beta, gamma, lambda, daysPerYear));
        ext::shared_ptr<PricingEngine> engine1 =
            MakeMCEuropeanGJRGARCHEngine<PseudoRandom>(process)
            .withStepsPerYear(20)
            .withAbsoluteTolerance(0.02)
            .withSeed(1234);

        ext::shared_ptr<PricingEngine> engine2(
            new AnalyticGJRGARCHEngine(ext::make_shared<GJRGARCHModel>(
                                               process)));
        for (Size i = 0; i < 2; ++i) {
            for (Size j = 0; j < 6; ++j) {
                Real x = strike[j];

                ext::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Call, x));
                Date exDate = today + maturity[i];
                ext::shared_ptr<Exercise> exercise(
                                                new EuropeanExercise(exDate));

                VanillaOption option(payoff, exercise);

                option.setPricingEngine(engine1);
                Real calculated = option.NPV();

                option.setPricingEngine(engine2);
                Real expected = option.NPV();
                Real tolerance = 7.5e-2;

                if (std::fabs(expected - analytic[k][i][j]) > 2.0*tolerance) {
                    BOOST_ERROR("failed to match results from engines"
                                << "\n    correct value:    "
                                << analytic[k][i][j]
                                << "\n    Analytic Approx.: "
                                << expected
                                << " +/- " << tolerance);
                }
                if (std::fabs(calculated-mcValues[k][i][j]) > 2.0*tolerance) {
                    BOOST_ERROR("failed to match results from engines"
                                << "\n    correct value:    "
                                << mcValues[k][i][j]
                                << "\n    Monte Carlo: " << calculated
                                << " +/- " << tolerance);
                }
            }
        }
    }
}


void GJRGARCHModelTest::testDAXCalibration() {
    /* this example is taken from A. Sepp
       Pricing European-Style Options under Jump Diffusion Processes
       with Stochstic Volatility: Applications of Fourier Transform
       http://math.ut.ee/~spartak/papers/stochjumpvols.pdf
    */

    BOOST_TEST_MESSAGE(
         "Testing GJR-GARCH model calibration using DAX volatility data...");

    Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = Actual365Fixed();
    Calendar calendar = TARGET();

    Integer t[] = { 13, 41, 75, 165, 256, 345, 524, 703 };
    Rate r[] = { 0.0357,0.0349,0.0341,0.0355,0.0359,0.0368,0.0386,0.0401 };

    std::vector<Date> dates;
    std::vector<Rate> rates;
    dates.push_back(settlementDate);
    rates.push_back(0.0357);
    Size i;
    for (i = 0; i < 8; ++i) {
        dates.push_back(settlementDate + t[i]);
        rates.push_back(r[i]);
    }
    Handle<YieldTermStructure> riskFreeTS(
                       ext::shared_ptr<YieldTermStructure>(
                                    new ZeroCurve(dates, rates, dayCounter)));

    Handle<YieldTermStructure> dividendTS(
                                   flatRate(settlementDate, 0.0, dayCounter));

    Volatility v[] =
      { 0.6625,0.4875,0.4204,0.3667,0.3431,0.3267,0.3121,0.3121,
        0.6007,0.4543,0.3967,0.3511,0.3279,0.3154,0.2984,0.2921,
        0.5084,0.4221,0.3718,0.3327,0.3155,0.3027,0.2919,0.2889,
        0.4541,0.3869,0.3492,0.3149,0.2963,0.2926,0.2819,0.2800,
        0.4060,0.3607,0.3330,0.2999,0.2887,0.2811,0.2751,0.2775,
        0.3726,0.3396,0.3108,0.2781,0.2788,0.2722,0.2661,0.2686,
        0.3550,0.3277,0.3012,0.2781,0.2781,0.2661,0.2661,0.2681,
        0.3428,0.3209,0.2958,0.2740,0.2688,0.2627,0.2580,0.2620,
        0.3302,0.3062,0.2799,0.2631,0.2573,0.2533,0.2504,0.2544,
        0.3343,0.2959,0.2705,0.2540,0.2504,0.2464,0.2448,0.2462,
        0.3460,0.2845,0.2624,0.2463,0.2425,0.2385,0.2373,0.2422,
        0.3857,0.2860,0.2578,0.2399,0.2357,0.2327,0.2312,0.2351,
        0.3976,0.2860,0.2607,0.2356,0.2297,0.2268,0.2241,0.2320 };

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(4468.17)));
    Real strike[] = { 3400,3600,3800,4000,4200,4400,
                      4500,4600,4800,5000,5200,5400,5600 };

    std::vector<ext::shared_ptr<CalibrationHelper> > options;

    const Real omega = 2.0e-6;
    const Real alpha = 0.024;
    const Real beta = 0.93;
    const Real gamma = 0.059;
    const Real lambda = 0.1;
    const Real daysPerYear = 365.0; // number of trading days per year
    const Real m1 = beta+(alpha+gamma*CumulativeNormalDistribution()(lambda))
            *(1.0+lambda*lambda)+gamma*lambda*std::exp(-lambda*lambda/2.0)
            /std::sqrt(2.0*M_PI);
    const Real v0 = omega/(1.0-m1);

    ext::shared_ptr<GJRGARCHProcess> process(new GJRGARCHProcess(
                             riskFreeTS, dividendTS, s0, v0,
                             omega, alpha, beta, gamma, lambda, daysPerYear));
    ext::shared_ptr<GJRGARCHModel> model(new GJRGARCHModel(process));

    ext::shared_ptr<PricingEngine> engine(
        new AnalyticGJRGARCHEngine(ext::shared_ptr<GJRGARCHModel>(model)));

    for (Size s = 3; s < 10; ++s) {
        for (Size m = 0; m < 3; ++m) {
            Handle<Quote> vol(ext::shared_ptr<Quote>(
                                                  new SimpleQuote(v[s*8+m])));

            Period maturity((int)((t[m]+3)/7.), Weeks); // round to weeks
            ext::shared_ptr<BlackCalibrationHelper> option(
                    new HestonModelHelper(maturity, calendar,
                                          s0->value(), strike[s], vol,
                                          riskFreeTS, dividendTS,
                                          BlackCalibrationHelper::ImpliedVolError));
            option->setPricingEngine(engine);
            options.push_back(option);
        }
    }

    Simplex om(0.05);
    model->calibrate(options, om,
                     EndCriteria(400, 40, 1.0e-8, 1.0e-8, 1.0e-8));

    Real sse = 0;
    for (i = 0; i < options.size(); ++i) {
        const Real diff = options[i]->calibrationError()*100.0;
        sse += diff*diff;
    }
    Real maxExpected = 15;
    if (sse > maxExpected) {
        BOOST_FAIL("Failed to reproduce calibration error"
                   << "\n    calculated: " << sse
                   << "\n    expected: < " << maxExpected);
    }
}

test_suite* GJRGARCHModelTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("GJR-GARCH model tests");

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&GJRGARCHModelTest::testDAXCalibration));
    }

    if (speed == Slow) {
        suite->add(QUANTLIB_TEST_CASE(&GJRGARCHModelTest::testEngines));
    }

    return suite;
}
