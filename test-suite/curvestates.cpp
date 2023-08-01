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

#include "curvestates.hpp"
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

namespace curve_states_test {

    struct CommonVars {
        // global data
        Date todaysDate, startDate, endDate;
        std::vector<Time> rateTimes, paymentTimes;
        std::vector<Real> accruals;
        Calendar calendar;
        DayCounter dayCounter;
        std::vector<Rate> todaysForwards, todaysCoterminalSwapRates;
        std::vector<Real> coterminalAnnuity;
        Spread displacement;
        std::vector<DiscountFactor> todaysDiscounts;

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
            paymentTimes = std::vector<Time>(rateTimes.size()-1);
            accruals = std::vector<Real>(rateTimes.size()-1);
            dayCounter = SimpleDayCounter();
            for (Size i=1; i<dates.size(); ++i)
                rateTimes[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);
            std::copy(rateTimes.begin()+1, rateTimes.end(),
                      paymentTimes.begin());
            for (Size i=1; i<rateTimes.size(); ++i)
                accruals[i-1] = rateTimes[i] - rateTimes[i-1];

            // Rates & displacement
            todaysForwards = std::vector<Rate>(paymentTimes.size());
            displacement = 0.0;
            for (Size i=0; i<todaysForwards.size(); ++i)
                todaysForwards[i] = 0.03 + 0.0010*i;

            // Discounts
            todaysDiscounts = std::vector<DiscountFactor>(rateTimes.size());
            todaysDiscounts[0] = 0.95;
            for (Size i=1; i<rateTimes.size(); ++i)
                todaysDiscounts[i] = todaysDiscounts[i-1] /
                    (1.0+todaysForwards[i-1]*accruals[i-1]);

            // Coterminal swap rates & annuities
            Size N = todaysForwards.size();
            todaysCoterminalSwapRates = std::vector<Rate>(N);
            coterminalAnnuity = std::vector<Real>(N);
            Real floatingLeg = 0.0;
            for (Size i=1; i<=N; ++i) {
                if (i==1) {
                    coterminalAnnuity[N-1] = accruals[N-1]*todaysDiscounts[N];
                } else {
                    coterminalAnnuity[N-i] = coterminalAnnuity[N-i+1] +
                        accruals[N-i]*todaysDiscounts[N-i+1];
                }
                floatingLeg = todaysDiscounts[N-i]-todaysDiscounts[N];
                todaysCoterminalSwapRates[N-i] =
                    floatingLeg/coterminalAnnuity[N-i];
            }

            std::vector<Time> evolutionTimes(rateTimes.size()-1);
            std::copy(rateTimes.begin(), rateTimes.end()-1,
                      evolutionTimes.begin());
            EvolutionDescription evolution(rateTimes,evolutionTimes);
            evolution.rateTaus();
            evolution.firstAliveRate();
        }
    };

}


void CurveStatesTest::testLMMCurveState() {

    BOOST_TEST_MESSAGE("Testing Libor-market-model curve state...");

    using namespace curve_states_test;

    CommonVars vars;
}

void CurveStatesTest::testCoterminalSwapCurveState() {

    BOOST_TEST_MESSAGE("Testing coterminal-swap-market-model curve state...");

    using namespace curve_states_test;

    CommonVars vars;
}


void CurveStatesTest::testCMSwapCurveState() {

    BOOST_TEST_MESSAGE("Testing constant-maturity-swap-market-model curve state...");

    using namespace curve_states_test;

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

    //BOOST_TEST_MESSAGE( << "Rates\nTime\tValue:"<< std::endl;)
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

    LMMDriftCalculator lmmDriftcalculator(pseudo, displacements, taus,
                                          numeraire, alive);
    LMMCurveState lmmCs(rateTimes);
    lmmCs.setOnForwardRates(forwards);
    /*
    std::vector<Real> lmmDrifts(nbRates);

    std::cout << "drifts:"<< std::endl;
    std::cout << "LMM\t\tCMS"<< std::endl;
    for (Size i = 0; i<nbRates; ++i){
         std::cout << lmmDrifts[i] << "\t\t"<< cmsDrifts[i] << std::endl;
    }*/

//    const std::vector<Rate>& dfs = cs.discountRatios();
    //std::cout << "discounts ratios:"<< std::endl;
    //std::cout << "LMM\tCMS"<< std::endl;
    /*for (Size i = 0; i <nbRates; ++i){
        std::cout << lmmCs.discountRatio(i, nbRates) << "\t"<< cmsCs.discountRatio(i, nbRates) << std::endl;
    }*/
}

// --- Call the desired tests
test_suite* CurveStatesTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Curve States tests");
    //suite->add(QUANTLIB_TEST_CASE(&CurveStatesTest::testLMMCurveState));
    //suite->add(QUANTLIB_TEST_CASE(&CurveStatesTest::testCoterminalSwapCurveState));
    suite->add(QUANTLIB_TEST_CASE(&CurveStatesTest::testCMSwapCurveState));
    return suite;
}
