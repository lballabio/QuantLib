
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
#include <ql/PricingEngines/Swaption/blackswaptionengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    // global data

    Integer exercises[] = { 1, 2, 3, 5, 7, 10 };
    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    bool payFixed[] = { false, true };

    Date today_, settlement_;
    Real nominal_;
    Calendar calendar_;
    BusinessDayConvention rollingConvention_;
    Frequency fixedFrequency_, floatingFrequency_;
    DayCounter fixedDayCount_;
    bool fixedIsAdjusted_;
    boost::shared_ptr<Xibor> index_;
    Integer settlementDays_, fixingDays_;
    RelinkableHandle<TermStructure> termStructure_;

    // utilities

    boost::shared_ptr<SimpleSwap> makeSwap(const Date& start, Integer length, 
                                           Rate fixedRate, 
                                           Spread floatingSpread, 
                                           bool payFixed) {
        return boost::shared_ptr<SimpleSwap>(
            new SimpleSwap(payFixed,start,length,Years,
                           calendar_,rollingConvention_,nominal_,
                           fixedFrequency_,fixedRate,fixedIsAdjusted_,
                           fixedDayCount_,floatingFrequency_,index_,
                           fixingDays_,floatingSpread,termStructure_));
    }

    boost::shared_ptr<Swaption> makeSwaption(
                                    const boost::shared_ptr<SimpleSwap>& swap,
                                    const Date& exercise, 
                                    Volatility volatility) {
        boost::shared_ptr<Quote> vol_me(new SimpleQuote(volatility));
        RelinkableHandle<Quote> vol_rh(vol_me);
        boost::shared_ptr<BlackModel> model(
                                       new BlackModel(vol_rh,termStructure_));
        boost::shared_ptr<PricingEngine> engine(
                                              new BlackSwaptionEngine(model));
        return boost::shared_ptr<Swaption>(new Swaption(
                  swap,
                  boost::shared_ptr<Exercise>(new EuropeanExercise(exercise)),
                  termStructure_,
                  engine));
    }

    void initialize() {
        today_ = Date::todaysDate();
        settlementDays_ = 2;
        fixingDays_ = 2;
        nominal_ = 100.0;
        rollingConvention_ = ModifiedFollowing;
        fixedFrequency_ = Annual;
        floatingFrequency_ = Semiannual;
        fixedDayCount_ = Thirty360();
        fixedIsAdjusted_ = false;
        index_ = boost::shared_ptr<Xibor>(
                                     new Euribor(12/floatingFrequency_,Months,
                                                 termStructure_));
        calendar_ = index_->calendar();
        settlement_ = calendar_.advance(today_,settlementDays_,Days);
        termStructure_.linkTo(
          boost::shared_ptr<TermStructure>(new FlatForward(today_,settlement_,
                                                           0.05,Actual365())));
    }

}

void SwaptionTest::testStrikeDependency() {

    BOOST_MESSAGE("Testing swaption dependency on strike...");

    initialize();

    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                // store the results for different rates...
                std::vector<Real> values;
                for (Size l=0; l<LENGTH(strikes); l++) {
                    boost::shared_ptr<SimpleSwap> swap = 
                        makeSwap(startDate,lengths[j],strikes[l],
                                 0.0,payFixed[k]);
                    boost::shared_ptr<Swaption> swaption = 
                        makeSwaption(swap,exerciseDate,0.20);
                    values.push_back(swaption->NPV());
                }
                // and check that they go the right way
                if (payFixed[k]) {
                    std::vector<Real>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_FAIL(
                            "NPV is increasing with the strike "
                            "in a payer swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n],2) + "\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n+1]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n+1],2));
                    }
                } else {
                    std::vector<Real>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_FAIL(
                            "NPV is decreasing with the strike "
                            "in a receiver swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n],2) + "\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n+1]) +
                            " at strike: " +
                            RateFormatter::toString(strikes[n+1],2));
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSpreadDependency() {

    BOOST_MESSAGE("Testing swaption dependency on spread...");

    initialize();

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                // store the results for different rates...
                std::vector<Real> values;
                for (Size l=0; l<LENGTH(spreads); l++) {
                    boost::shared_ptr<SimpleSwap> swap = 
                        makeSwap(startDate,lengths[j],0.06,
                                 spreads[l],payFixed[k]);
                    boost::shared_ptr<Swaption> swaption = 
                        makeSwaption(swap,exerciseDate,0.20);
                    values.push_back(swaption->NPV());
                }
                // and check that they go the right way
                if (payFixed[k]) {
                    std::vector<Real>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_FAIL(
                            "NPV is decreasing with the spread "
                            "in a payer swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n],2) + "\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n+1]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n+1],2));
                    }
                } else {
                    std::vector<Real>::iterator it = 
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_FAIL(
                            "NPV is increasing with the spread "
                            "in a receiver swaption: \n"
                            "    exercise date: " +
                            DateFormatter::toString(exerciseDate) + "\n"
                            "    length: " + 
                            IntegerFormatter::toString(lengths[j]) + " years\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n],2) + "\n"
                            "    value: " +
                            DecimalFormatter::toString(values[n+1]) +
                            " for spread: " +
                            RateFormatter::toString(spreads[n+1],2));
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSpreadTreatment() {

    BOOST_MESSAGE("Testing swaption treatment of spread...");

    initialize();

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                for (Size l=0; l<LENGTH(spreads); l++) {
                    boost::shared_ptr<SimpleSwap> swap = 
                        makeSwap(startDate,lengths[j],0.06,
                                 spreads[l],payFixed[k]);
                    Spread correction = spreads[l] *
                                        swap->floatingLegBPS() / 
                                        swap->fixedLegBPS();
                    boost::shared_ptr<SimpleSwap> equivalentSwap =
                        makeSwap(startDate,lengths[j],0.06+correction,
                                 0.0,payFixed[k]);
                    boost::shared_ptr<Swaption> swaption1 =
                        makeSwaption(swap,exerciseDate,0.20);
                    boost::shared_ptr<Swaption> swaption2 =
                        makeSwaption(equivalentSwap,exerciseDate,0.20);
                    if (QL_FABS(swaption1->NPV()-swaption2->NPV()) > 1.0e-10)
                        BOOST_FAIL(
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
                            DecimalFormatter::toString(swaption1->NPV()) + "\n"
                            "    value of equivalent swaption: " +
                            DecimalFormatter::toString(swaption2->NPV()));
                }
            }
        }
    }
}

void SwaptionTest::testCachedValue() {

    BOOST_MESSAGE("Testing swaption value against cached value...");

    initialize();

    today_ = Date(13,March,2002);
    settlement_ = Date(15,March,2002);
    termStructure_.linkTo(
        boost::shared_ptr<TermStructure>(new FlatForward(today_,settlement_,
                                                         0.05,Actual365())));
    Date exerciseDate = calendar_.advance(settlement_,5,Years);
    Date startDate = calendar_.advance(exerciseDate,settlementDays_,Days);
    boost::shared_ptr<SimpleSwap> swap = makeSwap(startDate,10,0.06,0.0,true);
    boost::shared_ptr<Swaption> swaption = 
        makeSwaption(swap,exerciseDate,0.20);
    Real cachedNPV = 3.639365179345;

    if (QL_FABS(swaption->NPV()-cachedNPV) > 1.0e-11)
        BOOST_FAIL(
            "failed to reproduce cached swaption value:\n"
            "    calculated: " +
            DecimalFormatter::toString(swaption->NPV(),12) + "\n"
            "    expected:   " +
            DecimalFormatter::toString(cachedNPV,12));
}


test_suite* SwaptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption tests");
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testStrikeDependency));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testSpreadDependency));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testSpreadTreatment));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testCachedValue));
    return suite;
}

