/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2006 Cristina Duminuco

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

#include "swaption.hpp"
#include "utilities.hpp"
#include <ql/Instruments/swaption.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/PricingEngines/Swaption/blackswaptionengine.hpp>
#include <ql/Utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwaptionTest)

// global data

Integer exercises[] = { 1, 2, 3, 5, 7, 10 };
Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
bool payFixed[] = { false, true };

Date today_, settlement_;
Real nominal_;
Calendar calendar_;
BusinessDayConvention fixedConvention_, floatingConvention_;
Frequency fixedFrequency_, floatingFrequency_;
DayCounter fixedDayCount_;
boost::shared_ptr<Xibor> index_;
Integer settlementDays_, fixingDays_;
Handle<YieldTermStructure> termStructure_;

// utilities

boost::shared_ptr<VanillaSwap> makeSwap(const Date& start, Integer length,
                                        Rate fixedRate,
                                        Spread floatingSpread,
                                        bool payFixed) {
    Date maturity = calendar_.advance(start,length,Years,
                                      floatingConvention_);
    Schedule fixedSchedule(calendar_,start,maturity,
                           fixedFrequency_,fixedConvention_);
    Schedule floatSchedule(calendar_,start,maturity,
                           floatingFrequency_,floatingConvention_);
    return boost::shared_ptr<VanillaSwap>(
            new VanillaSwap(payFixed,nominal_,
                            fixedSchedule,fixedRate,fixedDayCount_,
                            floatSchedule,index_,fixingDays_,floatingSpread,
                            index_->dayCounter(),termStructure_));
}

boost::shared_ptr<Swaption> makeSwaption(const boost::shared_ptr<VanillaSwap>& swap,
                                         const Date& exercise, Volatility volatility,
                                         Settlement::Type type = Settlement::Physical) {
    boost::shared_ptr<Quote> vol_me(new SimpleQuote(volatility));
    Handle<Quote> vol_rh(vol_me);
    boost::shared_ptr<BlackModel> model(new BlackModel(vol_rh));
    boost::shared_ptr<PricingEngine> engine(new BlackSwaptionEngine(model));
    return boost::shared_ptr<Swaption>(new Swaption(
                  swap,
                  boost::shared_ptr<Exercise>(new EuropeanExercise(exercise)),
                  termStructure_,
                  engine,
                  type));
}

void setup() {
    settlementDays_ = 2;
    fixingDays_ = 2;
    nominal_ = 100.0;
    fixedConvention_ = Unadjusted;
    floatingConvention_ = ModifiedFollowing;
    fixedFrequency_ = Annual;
    floatingFrequency_ = Semiannual;
    fixedDayCount_ = Thirty360();
    index_ = boost::shared_ptr<Xibor>(new Euribor(Period(12/floatingFrequency_,
                                                         Months),
                                                  termStructure_));
    calendar_ = index_->calendar();
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365Fixed()));  // by default: COntinuous and annual
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(SwaptionTest)


void SwaptionTest::testStrikeDependency() {

    BOOST_MESSAGE("Testing swaption dependency on strike...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

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
                std::vector<Real> values_cash;
                for (Size l=0; l<LENGTH(strikes); l++) {
                    boost::shared_ptr<VanillaSwap> swap =
                        makeSwap(startDate,lengths[j],strikes[l],
                                 0.0,payFixed[k]);
                    boost::shared_ptr<Swaption> swaption =
                        makeSwaption(swap,exerciseDate,0.20);
                    values.push_back(swaption->NPV());
                    boost::shared_ptr<Swaption> swaption_cash =
                        makeSwaption(swap,exerciseDate,0.20,Settlement::Cash);
                    values_cash.push_back(swaption->NPV());
                }
                // and check that they go the right way
                if (payFixed[k]) {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR(
                            "NPV is increasing with the strike "
                            << "in a payer swaption (physical delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values[n]
                            << " at strike: " << io::rate(strikes[n]) << "\n"
                            << "    value:  " << values[n+1]
                            << " at strike: " << io::rate(strikes[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::less<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR(
                           "NPV is increasing with the strike "
                            << "in a payer swaption (cash delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values_cash[n]
                            << " at strike: " << io::rate(strikes[n]) << "\n"
                            << "    value:  " << values_cash[n+1]
                            << " at strike: " << io::rate(strikes[n+1]));
                    }
                } else {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR(
                            "NPV is decreasing with the strike "
                            << "in a receiver swaption (physical delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values[n]
                            << " at strike: " << io::rate(strikes[n]) << "\n"
                            << "    value:  " << values[n+1]
                            << " at strike: " << io::rate(strikes[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::greater<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR(
                            "NPV is decreasing with the strike "
                            << "in a receiver swaption (cash delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values_cash[n]
                            << " at strike: " << io::rate(strikes[n]) << "\n"
                            << "    value:  " << values_cash[n+1]
                            << " at strike: " << io::rate(strikes[n+1]));
                    }
                }
            }
        }
    }

    QL_TEST_TEARDOWN
}

void SwaptionTest::testSpreadDependency() {

    BOOST_MESSAGE("Testing swaption dependency on spread...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

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
                std::vector<Real> values_cash;
                for (Size l=0; l<LENGTH(spreads); l++) {
                    boost::shared_ptr<VanillaSwap> swap =
                        makeSwap(startDate,lengths[j],0.06,
                                 spreads[l],payFixed[k]);
                    boost::shared_ptr<Swaption> swaption =
                        makeSwaption(swap,exerciseDate,0.20);
                    values.push_back(swaption->NPV());
                    boost::shared_ptr<Swaption> swaption_cash =
                        makeSwaption(swap,exerciseDate,0.20,Settlement::Cash);
                    values_cash.push_back(swaption->NPV());
                }
                // and check that they go the right way
                if (payFixed[k]) {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR(
                            "NPV is decreasing with the spread "
                            << "in a payer swaption (physical delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values[n]
                            << " for spread: " << io::rate(spreads[n]) << "\n"
                            << "    value:  " << values[n+1]
                            << " for spread: " << io::rate(spreads[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::greater<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR(
                            "NPV is decreasing with the spread "
                            << "in a payer swaption (cash delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values_cash[n]
                            << " for spread: " << io::rate(spreads[n]) << "\n"
                            << "    value:  " << values_cash[n+1]
                            << " for spread: " << io::rate(spreads[n+1]));
                    }
                } else {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR(
                            "NPV is increasing with the spread "
                            << "in a receiver swaption (physical delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values[n]
                            << " for spread: " << io::rate(spreads[n]) << "\n"
                            << "    value:  " << values[n+1]
                            << " for spread: " << io::rate(spreads[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::less<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR(
                            "NPV is increasing with the spread "
                            << "in a receiver swaption (cash delivered): \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    value:  " << values_cash[n]
                            << " for spread: " << io::rate(spreads[n]) << "\n"
                            << "    value:  " << values_cash[n+1]
                            << " for spread: " << io::rate(spreads[n+1]));
                    }
                }
            }
        }
    }

    QL_TEST_TEARDOWN
}

void SwaptionTest::testSpreadTreatment() {

    BOOST_MESSAGE("Testing swaption treatment of spread...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(payFixed); k++) {
                Date exerciseDate = calendar_.advance(today_,
                                                      exercises[i],Years);
                Date startDate = calendar_.advance(exerciseDate,
                                                   settlementDays_,Days);
                for (Size l=0; l<LENGTH(spreads); l++) {
                    boost::shared_ptr<VanillaSwap> swap =
                        makeSwap(startDate,lengths[j],0.06,
                                 spreads[l],payFixed[k]);
                    Spread correction = spreads[l] *
                                        swap->floatingLegBPS() /
                                        swap->fixedLegBPS();
                    boost::shared_ptr<VanillaSwap> equivalentSwap =
                        makeSwap(startDate,lengths[j],0.06+correction,
                                 0.0,payFixed[k]);
                    boost::shared_ptr<Swaption> swaption1 =
                        makeSwaption(swap,exerciseDate,0.20);
                    boost::shared_ptr<Swaption> swaption2 =
                        makeSwaption(equivalentSwap,exerciseDate,0.20);
                    boost::shared_ptr<Swaption> swaption1_cash =
                        makeSwaption(swap,exerciseDate,0.20,
                                     Settlement::Cash);
                    boost::shared_ptr<Swaption> swaption2_cash =
                        makeSwaption(equivalentSwap,exerciseDate,0.20,
                                     Settlement::Cash);
                    if (std::fabs(swaption1->NPV()-swaption2->NPV()) > 1.0e-10)
                        BOOST_ERROR(
                            "wrong spread treatment: \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    pay " << (payFixed[k] ? "fixed\n"
                                                          : "floating\n")
                            << "    spread: " << io::rate(spreads[l]) << "\n"
                            << "    value of original swaption:   "
                            << swaption1->NPV() << "\n"
                            << "    value of equivalent swaption: "
                            << swaption2->NPV());

                    if (std::fabs(swaption1_cash->NPV()-swaption2_cash->NPV()) > 1.0e-10)
                        BOOST_ERROR(
                            "wrong spread treatment: \n"
                            << "    exercise date: " << exerciseDate << "\n"
                            << "    length: " << lengths[j] << " years\n"
                            << "    pay " << (payFixed[k] ? "fixed\n"
                                                          : "floating\n")
                            << "    spread: " << io::rate(spreads[l]) << "\n"
                            << "    value of original swaption:   "
                            << swaption1_cash->NPV() << "\n"
                            << "    value of equivalent swaption: "
                            << swaption2_cash->NPV());
                }

            }
        }
    }

    QL_TEST_TEARDOWN
}

void SwaptionTest::testCachedValue() {

    BOOST_MESSAGE("Testing swaption value against cached value...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    today_ = Date(13,March,2002);
    settlement_ = Date(15,March,2002);
    Settings::instance().evaluationDate() = today_;
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365Fixed()));
    Date exerciseDate = calendar_.advance(settlement_,5,Years);
    Date startDate = calendar_.advance(exerciseDate,settlementDays_,Days);
    boost::shared_ptr<VanillaSwap> swap = makeSwap(startDate,10,0.06,0.0,true);
    boost::shared_ptr<Swaption> swaption =
        makeSwaption(swap,exerciseDate,0.20);
    #ifndef QL_USE_INDEXED_COUPON
    Real cachedNPV = 3.639365179345;
    #else
    Real cachedNPV = 3.639692232647;
    #endif

    if (std::fabs(swaption->NPV()-cachedNPV) > 1.0e-11)
        BOOST_ERROR("failed to reproduce cached swaption value:\n"
                    << QL_FIXED << std::setprecision(12)
                    << "    calculated: " << swaption->NPV() << "\n"
                    << "    expected:   " << cachedNPV);

    QL_TEST_TEARDOWN
}


void SwaptionTest::testCashSettledSwaptions() {

    BOOST_MESSAGE("Testing cash settled swaptions modified annuity...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Rate strike = 0.05;

    for (Size i=0; i<LENGTH(exercises); i++) {
        Integer Ex = exercises[i];
        for (Size j=0; j<LENGTH(lengths); j++) {
            Integer L = lengths[j];

            Date exerciseDate = calendar_.advance(today_,Ex,Years);
            Date startDate = calendar_.advance(exerciseDate,settlementDays_,Days);
            Date maturity = calendar_.advance(startDate,L,Years,floatingConvention_);
            Schedule floatSchedule(calendar_,startDate,maturity,floatingFrequency_,floatingConvention_);

            // Swap with fixed leg conventions: Business Days = Unadjusted, DayCount = 30/360
            Schedule fixedSchedule_u(calendar_,startDate,maturity,
                                     fixedFrequency_,Unadjusted,
                                     Date(),false,true);

            boost::shared_ptr<VanillaSwap> swap_u360(
                                    new VanillaSwap(payFixed[0], nominal_,
                                    fixedSchedule_u,strike,Thirty360(),
                                    floatSchedule,index_,fixingDays_,0.0,
                                    index_->dayCounter(),termStructure_));

            // Swap with fixed leg conventions: Business Days = Unadjusted, DayCount = Act/365
            boost::shared_ptr<VanillaSwap> swap_u365(
                                    new VanillaSwap(payFixed[0],nominal_,
                                    fixedSchedule_u,strike,Actual365Fixed(),
                                    floatSchedule,index_,fixingDays_,0.0,
                                    index_->dayCounter(),termStructure_));

            // Swap with fixed leg conventions: Business Days = Modified Following, DayCount = 30/360
            Schedule fixedSchedule_a(calendar_,startDate,maturity,
                                     fixedFrequency_,ModifiedFollowing,
                                     Date(),false,true);
            boost::shared_ptr<VanillaSwap> swap_a360(
                                    new VanillaSwap(payFixed[0],nominal_,
                                    fixedSchedule_a,strike,Thirty360(),
                                    floatSchedule,index_,fixingDays_,0.0,
                                    index_->dayCounter(),termStructure_));

            // Swap with fixed leg conventions: Business Days = Modified Following, DayCount = Act/365
            boost::shared_ptr<VanillaSwap> swap_a365(
                                    new VanillaSwap(payFixed[0],nominal_,
                                    fixedSchedule_a,strike,Actual365Fixed(),
                                    floatSchedule,index_,fixingDays_,0.0,
                                    index_->dayCounter(),termStructure_));

            const std::vector<boost::shared_ptr<CashFlow> >& swapFixedLeg_u360 = swap_u360->fixedLeg();
            const std::vector<boost::shared_ptr<CashFlow> >& swapFixedLeg_a360 = swap_a360->fixedLeg();
            const std::vector<boost::shared_ptr<CashFlow> >& swapFixedLeg_u365 = swap_u365->fixedLeg();
            const std::vector<boost::shared_ptr<CashFlow> >& swapFixedLeg_a365 = swap_a365->fixedLeg();

            // FlatForward curves
            boost::shared_ptr<YieldTermStructure> flatRate_u360(
                new FlatForward(settlement_,swap_u360->fairRate(),
                                Thirty360(),Compounded,fixedFrequency_));
            boost::shared_ptr<YieldTermStructure> flatRate_a360(
                new FlatForward(settlement_,swap_a360->fairRate(),
                                Thirty360(),Compounded,fixedFrequency_));
            boost::shared_ptr<YieldTermStructure> flatRate_u365(
                new FlatForward(settlement_,swap_u365->fairRate(),
                                Actual365Fixed(),Compounded,fixedFrequency_));
            boost::shared_ptr<YieldTermStructure> flatRate_a365(
                new FlatForward(settlement_,swap_a365->fairRate(),
                                Actual365Fixed(),Compounded,fixedFrequency_));

            Handle<YieldTermStructure> termStructure_u360;
            Handle<YieldTermStructure> termStructure_a360;
            Handle<YieldTermStructure> termStructure_u365;
            Handle<YieldTermStructure> termStructure_a365;
            termStructure_u360.linkTo(flatRate_u360);
            termStructure_a360.linkTo(flatRate_a360);
            termStructure_u365.linkTo(flatRate_u365);
            termStructure_a365.linkTo(flatRate_a365);

            // Annuity calculated by swap method fixedLegBPS().
            // Fixed leg conventions: Unadjusted, 30/360
            Real annuity_u360 = swap_u360->fixedLegBPS() / 0.0001;
            annuity_u360 = swap_u360->payFixedRate() ? -annuity_u360 :
                                                        annuity_u360 ;
            // Fixed leg conventions: ModifiedFollowing, act/365
            Real annuity_a365 = swap_a365->fixedLegBPS() / 0.0001;
            annuity_a365 = swap_a365->payFixedRate() ? -annuity_a365 :
                                                        annuity_a365 ;
            // Fixed leg conventions: ModifiedFollowing, 30/360
            Real annuity_a360 = swap_a360->fixedLegBPS() / 0.0001;
            annuity_a360 = swap_a360->payFixedRate() ? -annuity_a360 :
                                                        annuity_a360 ;
            // Fixed leg conventions: Unadjusted, act/365
            Real annuity_u365 = swap_u365->fixedLegBPS() / 0.0001;
            annuity_u365 = swap_u365->payFixedRate() ? -annuity_u365 :
                                                        annuity_u365 ;

            // Calculation of Modified Annuity (cash settlement)
            // Fixed leg conventions of swap: unadjusted, 30/360
            Real cashannuity_u360 = 0.;
            for (Size i=0; i<swapFixedLeg_u360.size(); i++) {
                cashannuity_u360 += swapFixedLeg_u360[i]->amount()/strike
                                  * termStructure_u360->discount(
                                    swapFixedLeg_u360[i]->date());
            }
            // Fixed leg conventions of swap: unadjusted, act/365
            Real cashannuity_u365 = 0.;
            for (Size i=0; i<swapFixedLeg_u365.size(); i++) {
                cashannuity_u365 += swapFixedLeg_u365[i]->amount()/strike
                                  * termStructure_u365->discount(
                                    swapFixedLeg_u365[i]->date());
            }
            // Fixed leg conventions of swap: modified following, 30/360
            Real cashannuity_a360 = 0.;
            for (Size i=0; i<swapFixedLeg_a360.size(); i++) {
                cashannuity_a360 += swapFixedLeg_a360[i]->amount()/strike
                                  * termStructure_a360->discount(
                                    swapFixedLeg_a360[i]->date());
            }
            // Fixed leg conventions of swap: modified following, act/365
            Real cashannuity_a365 = 0.;
            for (Size i=0; i<swapFixedLeg_a365.size(); i++) {
                cashannuity_a365 += swapFixedLeg_a365[i]->amount()/strike
                                  * termStructure_a365->discount(
                                    swapFixedLeg_a365[i]->date());
            }

            // Swaptions: underlying swap fixed leg conventions: unadjusted, 30/360
            // Physical settled swaption
            boost::shared_ptr<Swaption> swaption_p_u360 =
                                        makeSwaption(swap_u360,maturity,0.20);
            Real value_p_u360 = swaption_p_u360->NPV();
            // Cash settled swaption
            boost::shared_ptr<Swaption> swaption_c_u360 =
                                        makeSwaption(swap_u360,maturity,0.20,
                                        Settlement::Cash);
            Real value_c_u360 = swaption_c_u360->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_u360 = value_c_u360 / value_p_u360;
            Real annuity_ratio_u360 = cashannuity_u360 / annuity_u360;

            // Swaptions: underlying swap fixed leg conventions: modified following, act/365
            // Physical settled swaption
            boost::shared_ptr<Swaption> swaption_p_a365 =
                                        makeSwaption(swap_a365,maturity,0.20);
            Real value_p_a365 = swaption_p_a365->NPV();
            // Cash settled swaption
            boost::shared_ptr<Swaption> swaption_c_a365 =
                                        makeSwaption(swap_a365,maturity,0.20,
                                        Settlement::Cash);
            Real value_c_a365 = swaption_c_a365->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_a365 = value_c_a365 / value_p_a365;
            Real annuity_ratio_a365 =  cashannuity_a365 / annuity_a365;

            // Swaptions: underlying swap fixed leg conventions: modified following, 30/360
            // Physical settled swaption
            boost::shared_ptr<Swaption> swaption_p_a360 =
                                        makeSwaption(swap_a360,maturity,0.20);
            Real value_p_a360 = swaption_p_a360->NPV();
            // Cash settled swaption
            boost::shared_ptr<Swaption> swaption_c_a360 =
                                        makeSwaption(swap_a360,maturity,0.20,
                                        Settlement::Cash);
            Real value_c_a360 = swaption_c_a360->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_a360 = value_c_a360 / value_p_a360;
            Real annuity_ratio_a360 =  cashannuity_a360 / annuity_a360;

            // Swaptions: underlying swap fixed leg conventions: unadjusted, act/365
            // Physical settled swaption
            boost::shared_ptr<Swaption> swaption_p_u365 =
                                        makeSwaption(swap_u365,maturity,0.20);
            Real value_p_u365 = swaption_p_u365->NPV();
            // Cash settled swaption
            boost::shared_ptr<Swaption> swaption_c_u365 =
                                        makeSwaption(swap_u365,maturity,0.20,
                                        Settlement::Cash);
            Real value_c_u365 = swaption_c_u365->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_u365 = value_c_u365 / value_p_u365;
            Real annuity_ratio_u365 =  cashannuity_u365 / annuity_u365;

            if ( std::abs(annuity_ratio_u360-npv_ratio_u360)>1e-10 ) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            Ex << "y x " << L << "y" <<
                            " (underlying swap fixed leg Unadjusted, 30/360)" << "\n" <<
                            "    Today           : " <<
                            today_ << "\n" <<
                            "    Settlement date : " <<
                            settlement_ << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate << "\n"   <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : " <<
                            value_p_u360 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_u360 << "\n" <<
                            "    cash delivered swaption npv :     " <<
                            value_c_u360 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_u360 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_u360 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_u360 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_u360-npv_ratio_u360) );
            }
            if ( std::abs(annuity_ratio_a365-npv_ratio_a365)>1e-10 ) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            Ex << "y x " << L << "y" <<
                            " (underlying swap fixed leg Modified Following, act/365" << "\n" <<
                            "    Today           : " <<
                            today_ << "\n" <<
                            "    Settlement date : " <<
                            settlement_ << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate <<  "\n"  <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : "  <<
                            value_p_a365 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_a365 << "\n" <<
                            "    cash delivered swaption npv :     "  <<
                            value_c_a365 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_a365 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_a365 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_a365 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_a365-npv_ratio_a365) );
                }
            if ( std::abs(annuity_ratio_a360-npv_ratio_a360)>1e-10 ) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            Ex << "y x " << L << "y" <<
                            " (underlying swap fixed leg Unadjusted, 30/360)" << "\n" <<
                            "    Today           : " <<
                            today_ << "\n" <<
                            "    Settlement date : " <<
                            settlement_ << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate << "\n"   <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : " <<
                            value_p_a360 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_a360 << "\n" <<
                            "    cash delivered swaption npv :     " <<
                            value_c_a360 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_a360 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_a360 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_a360 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_a360-npv_ratio_a360) );
            }
            if ( std::abs(annuity_ratio_u365-npv_ratio_u365)>1e-10 ) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            Ex << "y x " << L << "y" <<
                            " (underlying swap fixed leg Unadjusted, act/365)" << "\n" <<
                            "    Today           : " <<
                            today_ << "\n" <<
                            "    Settlement date : " <<
                            settlement_ << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate << "\n"   <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : " <<
                            value_p_u365 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_u365 << "\n" <<
                            "    cash delivered swaption npv :     " <<
                            value_c_u365 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_u365 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_u365 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_u365 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_u365-npv_ratio_u365) );
            }
        }
    }

    QL_TEST_TEARDOWN

}

test_suite* SwaptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption tests");
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testCashSettledSwaptions));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testStrikeDependency));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testSpreadDependency));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testSpreadTreatment));
    suite->add(BOOST_TEST_CASE(&SwaptionTest::testCachedValue));
    return suite;
}

