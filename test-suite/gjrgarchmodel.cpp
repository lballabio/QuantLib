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
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/time/period.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void GJRGARCHModelTest::testEngines() {
    BOOST_MESSAGE(
       "Testing Monte Carlo GJR-GARCH engine against "
       "analytic GJR-GARCH engine...");

    DayCounter dayCounter = ActualActual();

    const Date today = Date::todaysDate();
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, 0.05, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(today, 0.0, dayCounter));

    const Real s0 = 50.0;
    const Real omega = 2.0e-6;
    const Real alpha = 0.024;
    const Real beta = 0.93;
    const Real gamma = 0.059;
    const Real daysPerYr = 365.0; // number of trading days per year
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
        Handle<Quote> q(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
        boost::shared_ptr<GJRGARCHProcess> process(new GJRGARCHProcess(
            riskFreeTS, dividendTS, q, v0, omega, alpha, beta, gamma, lambda, daysPerYr));
        boost::shared_ptr<PricingEngine> engine1 =
            MakeMCEuropeanGJRGARCHEngine<PseudoRandom>(process)
            .withStepsPerYear(20)
            .withTolerance(0.02)
            .withSeed(1234);

        boost::shared_ptr<PricingEngine> engine2(
            new AnalyticGJRGARCHEngine(boost::shared_ptr<GJRGARCHModel>(
                                               new GJRGARCHModel(process))));
        for (Size i = 0; i < 2; ++i) {
            for (Size j = 0; j < 6; ++j) {
                Real x = strike[j];

                boost::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Call, x));
                Date exDate = today + maturity[i];
                boost::shared_ptr<Exercise> exercise(
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

test_suite* GJRGARCHModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("GJR-GARCH model tests");
    suite->add(BOOST_TEST_CASE(&GJRGARCHModelTest::testEngines));
    return suite;
}

