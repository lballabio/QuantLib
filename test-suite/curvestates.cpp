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
    std::vector<Rate> todaysForwards, todaysCoterminalSwapRates;
    std::vector<Real> coterminalAnnuity;
    std::vector<Spread> displacements;
    std::vector<DiscountFactor> todaysDiscounts;
    const Real tol = 1.0e-5;

    Size N, numeraire;
    Matrix pseudo;
    const Size alive = 0;

    const std::vector<Real> expected_drifts = {
        -0.0825792, -0.0787625, -0.0748546, -0.0708555,  -0.0667655, -0.0625846, -0.0583128,
        -0.0539504, -0.0494972, -0.0449536, -0.0403194,  -0.0355949, -0.0307801, -0.025875,
        -0.0208799, -0.0157948, -0.0106197, -0.00535471, 0};
    const std::vector<Real> expected_discount_ratios = {
        1.58379, 1.55274, 1.52154, 1.49025, 1.45888, 1.42748, 1.39607, 1.36468, 1.33335, 1.3021,
        1.27096, 1.23996, 1.20913, 1.17848, 1.14806, 1.11788, 1.08796, 1.05833, 1.029};
    const std::vector<Real> expected_forward_rates = {
        0.04, 0.041, 0.042, 0.043, 0.044, 0.045, 0.046, 0.047, 0.048, 0.049,
        0.05, 0.051, 0.052, 0.053, 0.054, 0.055, 0.056, 0.057, 0.058};

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
        taus = std::vector<Time>(N, .5);
        for (Size i=0; i<todaysForwards.size(); ++i)
            todaysForwards[i] = 0.04 + 0.0010*i;

        // Discounts
        todaysDiscounts = std::vector<DiscountFactor>(rateTimes.size());
        todaysDiscounts[0] = 0.95;
        for (Size i=1; i<rateTimes.size(); ++i)
            todaysDiscounts[i] = todaysDiscounts[i-1] /
                (1.0+todaysForwards[i-1]*accruals[i-1]);

        // Coterminal swap rates & annuities
        
        //todaysCoterminalSwapRates = std::vector<Rate>(N);
        //coterminalAnnuity = std::vector<Real>(N);
        //Real floatingLeg = 0.0;
        //for (Size i=1; i<=N; ++i) {
        //    if (i==1) {
        //        coterminalAnnuity[N-1] = accruals[N-1]*todaysDiscounts[N];
        //    } else {
        //        coterminalAnnuity[N-i] = coterminalAnnuity[N-i+1] +
        //            accruals[N-i]*todaysDiscounts[N-i+1];
        //    }
        //    floatingLeg = todaysDiscounts[N-i]-todaysDiscounts[N];
        //    todaysCoterminalSwapRates[N-i] =
        //        floatingLeg/coterminalAnnuity[N-i];
        //}

        //std::vector<Time> evolutionTimes(N);
        //std::copy(rateTimes.begin(), rateTimes.end()-1,
        //          evolutionTimes.begin());
        //EvolutionDescription evolution(rateTimes,evolutionTimes);
        //evolution.rateTaus();
        //evolution.firstAliveRate();
    }
};

 BOOST_AUTO_TEST_CASE(testLMMCurveState) {

     BOOST_TEST_MESSAGE("Testing Libor-market-model curve state...");

     CommonVars vars;

     //std::cout << "Rates\nTime\tValue:" << std::endl;
     //for (Size i = 0; i < vars.rateTimes.size() - 1; ++i) {
     //   std::cout << vars.rateTimes[i + 1] << "\t" << io::rate(vars.todaysForwards[i]) << std::endl;
     //}

     LMMDriftCalculator lmmDriftcalculator(vars.pseudo, vars.displacements, vars.taus, vars.numeraire, vars.alive);
     LMMCurveState lmmCs(vars.rateTimes);
     lmmCs.setOnForwardRates(vars.todaysForwards);

     std::vector<Real> lmmDrifts(vars.N);
     lmmDriftcalculator.compute(lmmCs, lmmDrifts);

    // std::cout << "LMM drifts" << std::endl;
    // for (Size i = 0; i < vars.N; ++i) {
    //    std::cout << lmmDrifts[i] << std::endl;
    // }

    // std::cout << "LMM discounts ratios:" << std::endl;
    // for (Size i = 0; i < vars.N; ++i) {
    //    std::cout << lmmCs.discountRatio(i, vars.N) << std::endl;
    // }

    //std::cout << "LMM forward rates:" << std::endl;
    // for (Size i = 0; i < vars.N; ++i) {
    //    std::cout << lmmCs.forwardRate(i) << std::endl;
    // }

     
    for (Size i = 0; i < vars.N; ++i) {
        if (std::fabs(lmmDrifts[i] - vars.expected_drifts[i]) > vars.tol){
            std::cout << lmmDrifts[i] << "\t\t" << vars.expected_drifts[i] << std::endl;
            BOOST_FAIL("LMM drifts mismatched");
        }

        if (std::fabs(lmmCs.discountRatio(i, vars.N) - vars.expected_discount_ratios[i]) >
            vars.tol) {
            std::cout << lmmCs.discountRatio(i, vars.N) << "\t\t"
                      << vars.expected_discount_ratios[i] << std::endl;
            BOOST_FAIL("LMM discount ratio mismatch");
        }

        if (std::fabs(lmmCs.forwardRate(i) - vars.expected_forward_rates[i]) > vars.tol) {
            std::cout << lmmCs.forwardRate(i) << "\t\t" << vars.expected_forward_rates[i]
                      << std::endl;
            BOOST_FAIL("LMM forward rate mismatch");
        }
     }

 }

 BOOST_AUTO_TEST_CASE(testCoterminalSwapCurveState) {

     BOOST_TEST_MESSAGE("Testing coterminal-swap-market-model curve state...");


     CommonVars vars;
 }


BOOST_AUTO_TEST_CASE(testCMSwapCurveState) {

    BOOST_TEST_MESSAGE("Testing constant-maturity-swap-market-model curve state...");

    CommonVars vars;

    Size nbRates = vars.todaysForwards.size();
    Size factors = nbRates;
    Matrix pseudo(nbRates, factors, 0.1);
    std::vector<Spread> displacements(nbRates, .0);
    std::vector<Time> rateTimes(nbRates+1);
    std::vector<Time> taus(nbRates, .5);
    std::vector<Rate> forwards(nbRates, 0.0);

    //std::cout << "rate value:"<< std::endl;

    for (Size i = 0; i < forwards.size(); ++i)
        forwards[i] = static_cast<Rate>(i)*.001+.04;

    for (Size i = 0; i < rateTimes.size(); ++i)
        rateTimes[i] = static_cast<Time>(i+1)*.5;

    //std::cout << "Rates\nTime\tValue:"<< std::endl;
    //for (Size i = 0; i < rateTimes.size()-1; ++i){
    //    std::cout << rateTimes[i+1] << "\t"<<io::rate(forwards[i]) << std::endl;
    //}

    Size numeraire = nbRates;
    Size alive = 0;

    Size spanningFwds = 1;

    CMSMMDriftCalculator cmsDriftcalculator(pseudo, displacements, taus,
                                            numeraire, alive, spanningFwds);

    CMSwapCurveState cmsCs(rateTimes, spanningFwds);
    cmsCs.setOnCMSwapRates(forwards);
    std::vector<Real> cmsDrifts(nbRates);
    cmsDriftcalculator.compute(cmsCs,cmsDrifts);

    std::cout << "drifts:" << std::endl;
    std::cout << "tCMS" << std::endl;
    for (Size i = 0; i < nbRates; ++i) {
        std::cout << cmsDrifts[i] << std::endl;
    }

    std::cout << "discounts ratios:" << std::endl;
    std::cout << "CMS" << std::endl;
    for (Size i = 0; i < nbRates; ++i) {
        std::cout << cmsCs.discountRatio(i, nbRates) << std::endl;
    }

        std::cout << "LMM forward rates:" << std::endl;
    for (Size i = 0; i < vars.N; ++i) {
        std::cout << cmsCs.forwardRate(i) << std::endl;
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
