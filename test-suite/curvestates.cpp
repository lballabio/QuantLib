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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "curvestates.hpp"
#include "utilities.hpp"

#include <ql/MarketModels/CurveStates/lmmcurvestate.hpp>
#include <ql/MarketModels/CurveStates/coterminalswapcurvestate.hpp>
#include <ql/MarketModels/CurveStates/cmswapcurvestate.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/schedule.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <iostream>
#include <sstream>

#if defined(BOOST_MSVC)
#include <float.h>
//namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CurveStatesTest)


#define BEGIN(x) (x+0)
#define END(x) (x+LENGTH(x))


Date todaysDate, startDate, endDate;
std::vector<Time> rateTimes, paymentTimes;
std::vector<Real> accruals;
Calendar calendar;
DayCounter dayCounter;
std::vector<Rate> todaysForwards, todaysCoterminalSwapRates;
std::vector<Real> coterminalAnnuity;
Spread displacement;
std::vector<DiscountFactor> todaysDiscounts;

void setup() {
    // Times
    calendar = NullCalendar();
    todaysDate = Settings::instance().evaluationDate();
    //startDate = todaysDate + 5*Years;
    endDate = todaysDate + 10*Years;
    Schedule dates(todaysDate, endDate, Period(Semiannual),
                   calendar, Following, Following, true, false);
    rateTimes = std::vector<Time>(dates.size()-1);
    paymentTimes = std::vector<Time>(rateTimes.size()-1);
    accruals = std::vector<Real>(rateTimes.size()-1);
    dayCounter = SimpleDayCounter();
    for (Size i=1; i<dates.size(); ++i)
        rateTimes[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);
    std::copy(rateTimes.begin()+1, rateTimes.end(), paymentTimes.begin());
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
        todaysCoterminalSwapRates[N-i] = floatingLeg/coterminalAnnuity[N-i];
    }
    
    std::vector<Time> evolutionTimes(rateTimes.size()-1);
    std::copy(rateTimes.begin(), rateTimes.end()-1, evolutionTimes.begin());
    EvolutionDescription evolution(rateTimes,evolutionTimes);
    std::vector<Real> rateTaus = evolution.rateTaus();
    std::vector<Size> alive = evolution.firstAliveRate();
    Size numberOfSteps = evolutionTimes.size();
};

QL_END_TEST_LOCALS(CurveStatesTest)

void CurveStatesTest::testLMMCurveState() {

    BOOST_MESSAGE("Testing LMMCurveState class"
                  "in a LIBOR market model...");
    QL_TEST_SETUP
}

void CurveStatesTest::testCoterminalSwapCurveState() {

    BOOST_MESSAGE("Testing CoterminalSwapCurveState class"
                  "in a Swap market model...");
    QL_TEST_SETUP
}


void CurveStatesTest::testCMSwapCurveState() {

    BOOST_MESSAGE("Testing CoterminalSwapCurveState class"
                  "in a Swap market model...");
    QL_TEST_SETUP
    Size nbRates = todaysForwards.size();
    Size factors = nbRates;
    Matrix pseudo(nbRates, factors, 1.0);
    std::vector<Spread> displacements(nbRates, .0);
    std::vector<Time> rateTimes(nbRates);
    std::vector<Time> taus(nbRates, .5);
    std::vector<Rate> forwards(nbRates-1, .04);
    for (Size i = 0; i < forwards.size(); ++i){
        rateTimes[i] = static_cast<Time>(i)*.05;
    }
    Size numeraire = nbRates; 
    Size alive = 0; 
    Size spanningFwds = 5;

    CMSMMDriftCalculator driftcalulator(pseudo, displacements, taus, numeraire,
                                        alive, spanningFwds);
    
    
    CMSwapCurveState cs(rateTimes, spanningFwds);
    cs.setOnCMSwapRates(forwards);
    std::vector<Real> drifts(nbRates);
    driftcalulator.compute(cs,drifts);
    const std::vector<Rate>& dfs = cs.discountRatios();
    for (Size i = 0; i <nbRates; ++i)
        std::cout << dfs[i] << std::endl;
}

// --- Call the desired tests
test_suite* CurveStatesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Curve States tests");
    suite->add(BOOST_TEST_CASE(&CurveStatesTest::testLMMCurveState));
    suite->add(BOOST_TEST_CASE(&CurveStatesTest::testCoterminalSwapCurveState));
    suite->add(BOOST_TEST_CASE(&CurveStatesTest::testCMSwapCurveState));
    return suite;
}
