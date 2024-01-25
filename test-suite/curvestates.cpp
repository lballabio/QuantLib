/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/models/marketmodels/curvestates/coterminalswapcurvestate.hpp>
#include <ql/models/marketmodels/curvestates/cmswapcurvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/driftcomputation/lmmdriftcalculator.hpp>
#include <ql/models/marketmodels/driftcomputation/cmsmmdriftcalculator.hpp>
#include <ql/models/marketmodels/driftcomputation/smmdriftcalculator.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/matrix.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <sstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CurveStatesTests)

struct CommonVars {
    // global data
    Date todaysDate, startDate, endDate;
    std::vector<Time> rateTimes, paymentTimes, taus;
    std::vector<Real> accruals;
    Calendar calendar;
    DayCounter dayCounter;
    std::vector<Rate> todaysForwards;
    std::vector<Spread> displacements;
    std::vector<DiscountFactor> todaysDiscounts;
    const Real tol = 1.0e-4;

    Size N, numeraire;
    Matrix pseudo;
    const Size spanningFwds = 1;

    std::vector<Size> firstAliveRates;

    const std::vector<Real> expectedDrifts = {
        -0.0825792, -0.0787625, -0.0748546, -0.0708555,  -0.0667655, -0.0625846, -0.0583128,
        -0.0539504, -0.0494972, -0.0449536, -0.0403194,  -0.0355949, -0.0307801, -0.025875,
        -0.0208799, -0.0157948, -0.0106197, -0.00535471, 0};

    const std::vector<Real> expectedDiscountRatios = {
        1.58379, 1.55274, 1.52154, 1.49025, 1.45888, 1.42748, 1.39607, 1.36468, 1.33335, 1.3021,
        1.27096, 1.23996, 1.20913, 1.17848, 1.14806, 1.11788, 1.08796, 1.05833, 1.029};

    const std::vector<Real> expectedForwardRates = {
        0.04, 0.041, 0.042, 0.043, 0.044, 0.045, 0.046, 0.047, 0.048, 0.049,
        0.05, 0.051, 0.052, 0.053, 0.054, 0.055, 0.056, 0.057, 0.058};

    const std::vector<Real> expectedSwapAnnuity = {
        0.776368, 0.760772, 0.745125, 0.729442, 0.713739, 0.698034, 0.68234,
        0.666673, 0.651048, 0.635479, 0.619979, 0.604563, 0.589242, 0.574031,
        0.558939, 0.54398,  0.529163, 0.5145,   0.5};

    const std::vector<Real> expectedCotDrifts = {
           -0.0472372, -0.0447452, -0.042233, -0.0397016, -0.0371516, -0.034584, -0.0319995, -0.0293991 ,
        -0.0267836, -0.0241539, -0.0215109, -0.0188555, -0.0161887, -0.0135113, -0.0108244 ,
        -0.00812878, -0.00542554, -0.00271562, 0};

    const std::vector<Real> expectedCotDiscountRatios = {
        1.58379, 1.55274, 1.52154, 1.49025, 1.45888, 1.42748, 1.39607, 1.36468, 1.33335, 1.3021,
        1.27096, 1.23996, 1.20913, 1.17848, 1.14806, 1.11788, 1.08796, 1.05833, 1.029};

    const std::vector<Real> expectedCotSwapAnnuity = {
        12.0934, 11.317,  10.5563, 9.81115, 9.08171, 8.36797, 7.66994, 6.9876, 6.32092, 5.66988,
        5.0344,  4.41442, 3.80986, 3.22061, 2.64658, 2.08764, 1.54366, 1.0145, 0.5};

    CommonVars() {
        // Times
        calendar = NullCalendar();
        todaysDate = Settings::instance().evaluationDate();
        //startDate = todaysDate + 5*Years;
        endDate = todaysDate + 10*Years;
        Schedule dates(todaysDate, endDate, Period(Semiannual),
                       calendar, Following, Following,
                       DateGeneration::Backward, false);
        rateTimes = std::vector<Time>(dates.size()-1);

        // init
        N = rateTimes.size() - 1;
        paymentTimes = std::vector<Time>(N);
        accruals = std::vector<Real>(N);
        numeraire = N;
        pseudo = Matrix(N, N, 0.1);

        dayCounter = SimpleDayCounter();
        for (Size i=1; i<dates.size(); ++i)
            rateTimes[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);
        std::copy(rateTimes.begin()+1, rateTimes.end(),
                  paymentTimes.begin());
        for (Size i=1; i<rateTimes.size(); ++i)
            accruals[i-1] = rateTimes[i] - rateTimes[i-1];

        // Rates & displacement
        todaysForwards = std::vector<Rate>(N);
        displacements = std::vector<Spread>(N, .0);
        //taus = std::vector<Time>(N, .5);
        for (Size i=0; i<todaysForwards.size(); ++i)
            todaysForwards[i] = 0.04 + 0.0010*i;

        // Discounts
        todaysDiscounts = std::vector<DiscountFactor>(rateTimes.size());
        todaysDiscounts[0] = 0.95;
        for (Size i=1; i<rateTimes.size(); ++i)
            todaysDiscounts[i] = todaysDiscounts[i-1] /
                (1.0+todaysForwards[i-1]*accruals[i-1]);

        // taus & first alive rates
        std::vector<Time> evolutionTimes(N);
        std::copy(rateTimes.begin(), rateTimes.end() - 1, evolutionTimes.begin());
        EvolutionDescription evolution(rateTimes, evolutionTimes);
        taus = evolution.rateTaus();
        firstAliveRates = evolution.firstAliveRate();

    }
};

 BOOST_AUTO_TEST_CASE(testLMMCurveState) {

     BOOST_TEST_MESSAGE("Testing Libor-market-model curve state...");

     CommonVars vars;

     LMMDriftCalculator lmmDriftcalculator(vars.pseudo, vars.displacements, vars.taus,
                                           vars.numeraire, vars.firstAliveRates[0]);
     LMMCurveState lmmCs(vars.rateTimes);
     lmmCs.setOnForwardRates(vars.todaysForwards);

     std::vector<Real> lmmDrifts(vars.N);
     lmmDriftcalculator.compute(lmmCs, lmmDrifts);
     
    for (Size i = 0; i < vars.N; ++i) {
        if (std::fabs(lmmDrifts[i] - vars.expectedDrifts[i]) > vars.tol){
            BOOST_FAIL("LMM drifts mismatched: " << 
                lmmDrifts[i] << "\t\t" << vars.expectedDrifts[i]);
        }

        if (std::fabs(lmmCs.discountRatio(i, vars.N) - vars.expectedDiscountRatios[i]) >
            vars.tol) {
            BOOST_FAIL("LMM discount ratio mismatch: " <<
                lmmCs.discountRatio(i, vars.N) << "\t\t" << vars.expectedDiscountRatios[i]);
        }

        if (std::fabs(lmmCs.forwardRate(i) - vars.expectedForwardRates[i]) > vars.tol) {
            BOOST_FAIL("LMM forward rate mismatch: " << lmmCs.forwardRate(i) << "\t\t"
                                                     << vars.expectedForwardRates[i]);
        }
     }

 }

 BOOST_AUTO_TEST_CASE(testCoterminalSwapCurveState) {

     BOOST_TEST_MESSAGE("Testing coterminal-swap-market-model curve state...");

     CommonVars vars;

    // Coterminal swap rates & annuities
     std::vector<Real> todaysCoterminalSwapRates = std::vector<Rate>(vars.N);
     std::vector<Real> coterminalAnnuity = std::vector<Real>(vars.N);
      Real floatingLeg = 0.0;
      for (Size i=1; i<=vars.N; ++i) {
          if (i==1) {
            coterminalAnnuity[vars.N - 1] =
                vars.accruals[vars.N - 1] * vars.todaysDiscounts[vars.N];
          } else {
            coterminalAnnuity[vars.N - i] =
                coterminalAnnuity[vars.N - i + 1] +
                vars.accruals[vars.N - i] * vars.todaysDiscounts[vars.N - i + 1];
          }
          floatingLeg = vars.todaysDiscounts[vars.N - i] - vars.todaysDiscounts[vars.N];
          todaysCoterminalSwapRates[vars.N - i] = floatingLeg / coterminalAnnuity[vars.N - i];
      }

      std::vector<Time> evolutionTimes(vars.N);
      std::copy(vars.rateTimes.begin(), vars.rateTimes.end() - 1,
                evolutionTimes.begin());
      EvolutionDescription evolution(vars.rateTimes, evolutionTimes);
      std::vector<Time> taus = evolution.rateTaus();
      std::vector<Size> alive = evolution.firstAliveRate();


    SMMDriftCalculator smmDriftcalculator(vars.pseudo, vars.displacements, taus, vars.numeraire,
                                            vars.firstAliveRates[0]);
      CoterminalSwapCurveState cotCs(vars.rateTimes);
    cotCs.setOnCoterminalSwapRates(todaysCoterminalSwapRates);

      std::vector<Real> cotDrifts(vars.N);
      smmDriftcalculator.compute(cotCs, cotDrifts);

     for (Size i = 0; i < vars.N; ++i) {
        if (std::fabs(cotDrifts[i] - vars.expectedCotDrifts[i]) > vars.tol) {
            BOOST_FAIL("COT drifts mismatched: " << cotDrifts[i] << "\t\t"
                                                 << vars.expectedCotDrifts[i]);
        }

        if (std::fabs(cotCs.discountRatio(i, vars.N) - vars.expectedCotDiscountRatios[i]) >
            vars.tol) {
            BOOST_FAIL("COT discount ratio mismatch: " << cotCs.discountRatio(i, vars.N) << "\t\t"
                                                       << vars.expectedCotDiscountRatios[i]);
        }

        if (std::fabs(cotCs.forwardRate(i) - vars.expectedForwardRates[i]) > vars.tol) {
            BOOST_FAIL("COT forward rate mismatch: " << cotCs.forwardRate(i) << "\t\t"
                                                     << vars.expectedForwardRates[i]);
        }

        if (std::fabs(cotCs.coterminalSwapRate(i) - todaysCoterminalSwapRates[i]) >
            vars.tol) {
            // Swap rate should be the same as Forward Rates
            BOOST_FAIL("COT swap rate mismatch: " << cotCs.coterminalSwapRate(i) << "\t\t"
                                                  << todaysCoterminalSwapRates[i]);
        }

        if (std::fabs(cotCs.coterminalSwapAnnuity(vars.numeraire, i) - 
            vars.expectedCotSwapAnnuity[i]) >
            vars.tol) {
            BOOST_FAIL("COT swap annunity mismatch: "
                       << cotCs.coterminalSwapAnnuity(vars.numeraire, i) << "\t\t"
                       << vars.expectedCotSwapAnnuity[i]);
        }
     }
 }


BOOST_AUTO_TEST_CASE(testCMSwapCurveState) {

    BOOST_TEST_MESSAGE("Testing constant-maturity-swap-market-model curve state...");

    CommonVars vars;

    CMSMMDriftCalculator cmsDriftcalculator(vars.pseudo, vars.displacements, vars.taus,
                                            vars.numeraire, vars.firstAliveRates[0],
                                            vars.spanningFwds);

    CMSwapCurveState cmsCs(vars.rateTimes, vars.spanningFwds);
    cmsCs.setOnCMSwapRates(vars.todaysForwards);
    std::vector<Real> cmsDrifts(vars.N);
    cmsDriftcalculator.compute(cmsCs,cmsDrifts);

    for (Size i = 0; i < vars.N; ++i) {
        if (std::fabs(cmsDrifts[i] - vars.expectedDrifts[i]) > vars.tol) {
            BOOST_FAIL("CMS drifts mismatched: " << cmsDrifts[i] << "\t\t"
                                                 << vars.expectedDrifts[i]);
        }

        if (std::fabs(cmsCs.discountRatio(i, vars.N) - vars.expectedDiscountRatios[i]) >
            vars.tol) {
            BOOST_FAIL("CMS discount ratio mismatch: " << cmsCs.discountRatio(i, vars.N) << "\t\t"
                                                       << vars.expectedDiscountRatios[i]);
        }

        if (std::fabs(cmsCs.forwardRate(i) - vars.expectedForwardRates[i]) > vars.tol) {
            BOOST_FAIL("CMS forward rate mismatch: " << cmsCs.forwardRate(i) << "\t\t"
                                                     << vars.expectedForwardRates[i]);
        }

        if (std::fabs(cmsCs.cmSwapRate(i, vars.spanningFwds) - vars.expectedForwardRates[i]) >
            vars.tol) {
            // Swap rate should be the same as Forward Rates
            BOOST_FAIL("CMS swap rate mismatch: " << cmsCs.cmSwapRate(i, vars.spanningFwds)
                                                  << "\t\t" << vars.expectedForwardRates[i]);
        }

        if (std::fabs(cmsCs.cmSwapAnnuity(vars.numeraire, i, vars.spanningFwds) -
                      vars.expectedSwapAnnuity[i]) >
            vars.tol) {
            BOOST_FAIL("CMS swap annunity mismatch: "
                       << cmsCs.cmSwapAnnuity(vars.numeraire, i, vars.spanningFwds) << "\t\t"
                       << vars.expectedSwapAnnuity[i]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
