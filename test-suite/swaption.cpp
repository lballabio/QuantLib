
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "swaption.hpp"
#include "utilities.hpp"
#include <ql/Instruments/swaption.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/PricingEngines/Swaption/blackswaption.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    // global data

    int exercises[] = { 1, 2, 3, 5, 7, 10 };
    int lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    bool payFixed[] = { false, true };

    Date today_, settlement_;
    double nominal_;
    Calendar calendar_;
    RollingConvention rollingConvention_;
    int fixedFrequency_, floatingFrequency_;
    DayCounter fixedDayCount_;
    bool fixedIsAdjusted_;
    Handle<Xibor> index_;
    int settlementDays_, fixingDays_;
    RelinkableHandle<TermStructure> termStructure_;

    // utilities

    Handle<SimpleSwap> makeSwap(const Date& start, int length, 
                                Rate fixedRate, Spread floatingSpread, 
                                bool payFixed) {
        return Handle<SimpleSwap>(
            new SimpleSwap(payFixed,start,length,Years,
                           calendar_,rollingConvention_,nominal_,
                           fixedFrequency_,fixedRate,fixedIsAdjusted_,
                           fixedDayCount_,floatingFrequency_,index_,
                           fixingDays_,floatingSpread,termStructure_));
    }

    Handle<Swaption> makeSwaption(const Handle<SimpleSwap>& swap,
                                  const Date& exercise, 
                                  double volatility) {
        Handle<Quote> vol_me(new SimpleQuote(volatility));
        RelinkableHandle<Quote> vol_rh(vol_me);
        Handle<BlackModel> model(new BlackModel(vol_rh,termStructure_));
        Handle<PricingEngine> engine(new BlackSwaption(model));
        return Handle<Swaption>(new Swaption(
            swap,
            Handle<Exercise>(new EuropeanExercise(exercise)),
            termStructure_,engine));
    }

}

// tests

void SwaptionTest::setUp() {
    today_ = Date::todaysDate();
    settlementDays_ = 2;
    fixingDays_ = 2;
    nominal_ = 100.0;
    rollingConvention_ = ModifiedFollowing;
    fixedFrequency_ = 1;
    floatingFrequency_ = 2;
    fixedDayCount_ = Thirty360();
    fixedIsAdjusted_ = false;
    index_ = Handle<Xibor>(new Euribor(12/floatingFrequency_,Months,
                                       termStructure_));
    calendar_ = index_->calendar();
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(
        Handle<TermStructure>(new FlatForward(today_,settlement_,0.05,
                                              Actual365())));
}

void SwaptionTest::testStrikeDependency() {

    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                // store the results for different rates...
                std::vector<double> values;
                for (Size l=0; l<LENGTH(strikes); l++) {
                    Handle<SimpleSwap> swap = 
                        makeSwap(startDate,lengths[j],strikes[l],
                                 0.0,payFixed[k]);
                    Handle<Swaption> swaption = 
                        makeSwaption(swap,exerciseDate,0.20);
                    values.push_back(swaption->NPV());
                }
                // and check that they go the right way
                if (payFixed[k]) {
                    std::vector<double>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<double>());
                    if (it != values.end()) {
                        int n = it - values.begin();
                        CPPUNIT_FAIL(
                            "NPV is increasing with the strike "
                            "in a payer swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n],2) + "\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n+1]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n+1],2));
                    }
                } else {
                    std::vector<double>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<double>());
                    if (it != values.end()) {
                        int n = it - values.begin();
                        CPPUNIT_FAIL(
                            "NPV is decreasing with the strike "
                            "in a receiver swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n],2) + "\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n+1]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n+1],2));
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSpreadDependency() {

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                // store the results for different rates...
                std::vector<double> values;
                for (Size l=0; l<LENGTH(spreads); l++) {
                    Handle<SimpleSwap> swap = 
                        makeSwap(startDate,lengths[j],0.06,
                                 spreads[l],payFixed[k]);
                    Handle<Swaption> swaption = 
                        makeSwaption(swap,exerciseDate,0.20);
                    values.push_back(swaption->NPV());
                }
                // and check that they go the right way
                if (payFixed[k]) {
                    std::vector<double>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<double>());
                    if (it != values.end()) {
                        int n = it - values.begin();
                        CPPUNIT_FAIL(
                            "NPV is decreasing with the spread "
                            "in a payer swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n],2) + "\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n+1]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n+1],2));
                    }
                } else {
                    std::vector<double>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<double>());
                    if (it != values.end()) {
                        int n = it - values.begin();
                        CPPUNIT_FAIL(
                            "NPV is increasing with the spread "
                            "in a receiver swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n],2) + "\n"
                            "    value: " +
                            DoubleFormatter::toString(values[n+1]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n+1],2));
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSpreadTreatment() {

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                for (Size l=0; l<LENGTH(spreads); l++) {
                    Handle<SimpleSwap> swap = 
                        makeSwap(startDate,lengths[j],0.06,
                                 spreads[l],payFixed[k]);
                    Spread correction = spreads[l] *
                                        swap->floatingLegBPS() / 
                                        swap->fixedLegBPS();
                    Handle<SimpleSwap> equivalentSwap =
                        makeSwap(startDate,lengths[j],0.06+correction,
                                 0.0,payFixed[k]);
                    Handle<Swaption> swaption1 =
                        makeSwaption(swap,exerciseDate,0.20);
                    Handle<Swaption> swaption2 =
                        makeSwaption(equivalentSwap,exerciseDate,0.20);
                    if (QL_FABS(swaption1->NPV()-swaption2->NPV()) > 1.0e-10)
                        CPPUNIT_FAIL(
                            "wrong spread treatment: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    pay " +
                            std::string(payFixed[k] ? "fixed\n" 
                                                    : "floating\n") +
                            "    spread: " +
                            RateFormatter::toString(spreads[l],2) + "\n"
                            "    value of original swaption:   " +
                            DoubleFormatter::toString(swaption1->NPV()) + "\n"
                            "    value of equivalent swaption: " +
                            DoubleFormatter::toString(swaption2->NPV()));
                }
            }
        }
    }
}

void SwaptionTest::testCachedValue() {

    today_ = Date(13,March,2002);
    settlement_ = Date(15,March,2002);
    termStructure_.linkTo(
        Handle<TermStructure>(new FlatForward(today_,settlement_,0.05,
                                              Actual365())));
    Date exerciseDate = calendar_.advance(settlement_,5,Years);
    Date startDate = calendar_.advance(exerciseDate,settlementDays_,Days);
    Handle<SimpleSwap> swap = makeSwap(startDate,10,0.06,0.0,true);
    Handle<Swaption> swaption = makeSwaption(swap,exerciseDate,0.20);
    double cachedNPV = 3.639365179345;
    
    if (QL_FABS(swaption->NPV()-cachedNPV) > 1.0e-11)
        CPPUNIT_FAIL(
            "failed to reproduce cached swaption value:\n"
            "    calculated: " +
            DoubleFormatter::toString(swaption->NPV(),12) + "\n"
            "    expected:   " +
            DoubleFormatter::toString(cachedNPV,12));
}

CppUnit::Test* SwaptionTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Swaption tests");
    tests->addTest(new CppUnit::TestCaller<SwaptionTest>
                   ("Testing swaption dependency on strike",
                    &SwaptionTest::testStrikeDependency));
    tests->addTest(new CppUnit::TestCaller<SwaptionTest>
                   ("Testing swaption dependency on spread",
                    &SwaptionTest::testSpreadDependency));
    tests->addTest(new CppUnit::TestCaller<SwaptionTest>
                   ("Testing swaption treatment of spread",
                    &SwaptionTest::testSpreadTreatment));
    tests->addTest(new CppUnit::TestCaller<SwaptionTest>
                   ("Testing swaption value against cached value",
                    &SwaptionTest::testCachedValue));
    return tests;
}

