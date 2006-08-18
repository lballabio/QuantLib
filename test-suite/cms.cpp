/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

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

#include "cms.hpp"
#include "utilities.hpp"
#include <ql/Instruments/vanillaswap.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/CashFlows/inarrearindexedcoupon.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/indexedcashflowvectors.hpp>
#include <ql/Volatilities/capletconstantvol.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/CashFlows/ConundrumPricer.hpp>
#include <iostream>
#include <ql/TermStructures/all.hpp>
#include <ql/Volatilities/swaptionvolmatrix.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwapTest)

// global data

Date today_, settlement_;
bool payFixed_;
Real nominal_;
Calendar calendar_;
BusinessDayConvention fixedConvention_, floatingConvention_;
Frequency fixedFrequency_, floatingFrequency_;
DayCounter fixedDayCount_;
boost::shared_ptr<Xibor> index_;
Integer settlementDays_;
Handle<YieldTermStructure> termStructure_;

// utilities

boost::shared_ptr<VanillaSwap> makeSwap(Integer length, Rate fixedRate,
                                        Spread floatingSpread) {
    Date maturity = calendar_.advance(settlement_,length,Years,
                                      floatingConvention_);
    Schedule fixedSchedule(calendar_,settlement_,maturity,
                           fixedFrequency_,fixedConvention_);
    Schedule floatSchedule(calendar_,settlement_,maturity,
                           floatingFrequency_,floatingConvention_);
    return boost::shared_ptr<VanillaSwap>(
            new VanillaSwap(payFixed_,nominal_,
                            fixedSchedule,fixedRate,fixedDayCount_,
                            floatSchedule,index_,floatingSpread,
                            index_->dayCounter(),termStructure_));
}

void setup() {
    payFixed_ = true;
    settlementDays_ = 2;
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
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365Fixed()));
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(SwapTest)


//void SwapTest::testFairRate() {
//
//    BOOST_MESSAGE("Testing simple swap calculation of fair fixed rate...");
//
//    QL_TEST_BEGIN
//    QL_TEST_SETUP
//
//    Integer lengths[] = { 1, 2, 5, 10, 20 };
//    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };
//
//    for (Size i=0; i<LENGTH(lengths); i++) {
//        for (Size j=0; j<LENGTH(spreads); j++) {
//
//            boost::shared_ptr<VanillaSwap> swap =
//                makeSwap(lengths[i],0.0,spreads[j]);
//            swap = makeSwap(lengths[i],swap->fairRate(),spreads[j]);
//            if (std::fabs(swap->NPV()) > 1.0e-10) {
//                BOOST_ERROR("recalculating with implied rate:\n"
//                            << std::setprecision(2)
//                            << "    length: " << lengths[i] << " years\n"
//                            << "    floating spread: "
//                            << io::rate(spreads[j]) << "\n"
//                            << "    swap value: " << swap->NPV());
//            }
//        }
//    }
//
//    QL_TEST_TEARDOWN
//}

void CmsTest::testFairRate()  {

	BOOST_MESSAGE("Testing constant maturity swap...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

	       Handle<YieldTermStructure> discountingTermStructure;

		{
		        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();
        // uncommenting the following line generates an error
        // calendar = Tokyo();
        Date settlementDate(22, September, 2004);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);

        Integer fixingDays = 2;
        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
        // nothing to do with Date::todaysDate
        Settings::instance().evaluationDate() = todaysDate;


        todaysDate = Settings::instance().evaluationDate();
        std::cout << "Today: " << todaysDate.weekday()
                  << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
                  << ", " << settlementDate << std::endl;

        // deposits
        Rate d1wQuote=0.0382;
        Rate d1mQuote=0.0372;
        Rate d3mQuote=0.0363;
        Rate d6mQuote=0.0353;
        Rate d9mQuote=0.0348;
        Rate d1yQuote=0.0345;
        // FRAs
        Rate fra3x6Quote=0.037125;
        Rate fra6x9Quote=0.037125;
        Rate fra6x12Quote=0.037125;
        // futures
        Real fut1Quote=96.2875;
        Real fut2Quote=96.7875;
        Real fut3Quote=96.9875;
        Real fut4Quote=96.6875;
        Real fut5Quote=96.4875;
        Real fut6Quote=96.3875;
        Real fut7Quote=96.2875;
        Real fut8Quote=96.0875;
        // swaps
        Rate s2yQuote=0.037125;
        Rate s3yQuote=0.0398;
        Rate s5yQuote=0.0443;
        Rate s10yQuote=0.05165;
        Rate s15yQuote=0.055175;


        /********************
         ***    QUOTES    ***
         ********************/

        // SimpleQuote stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.

        // deposits
        boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
        boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
        boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
        boost::shared_ptr<Quote> d6mRate(new SimpleQuote(d6mQuote));
        boost::shared_ptr<Quote> d9mRate(new SimpleQuote(d9mQuote));
        boost::shared_ptr<Quote> d1yRate(new SimpleQuote(d1yQuote));
        // FRAs
        boost::shared_ptr<Quote> fra3x6Rate(new SimpleQuote(fra3x6Quote));
        boost::shared_ptr<Quote> fra6x9Rate(new SimpleQuote(fra6x9Quote));
        boost::shared_ptr<Quote> fra6x12Rate(new SimpleQuote(fra6x12Quote));
        // futures
        boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));
        boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));
        boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));
        boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));
        boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));
        boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));
        boost::shared_ptr<Quote> fut7Price(new SimpleQuote(fut7Quote));
        boost::shared_ptr<Quote> fut8Price(new SimpleQuote(fut8Quote));
        // swaps
        boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
        boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
        boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
        boost::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));
        boost::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));


        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // deposits
        DayCounter depositDayCounter = Actual360();

        boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(
            Handle<Quote>(d1wRate),
            1*Weeks, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(
            Handle<Quote>(d1mRate),
            1*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(
            Handle<Quote>(d3mRate),
            3*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d6m(new DepositRateHelper(
            Handle<Quote>(d6mRate),
            6*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d9m(new DepositRateHelper(
            Handle<Quote>(d9mRate),
            9*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d1y(new DepositRateHelper(
            Handle<Quote>(d1yRate),
            1*Years, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));


        // setup FRAs
        boost::shared_ptr<RateHelper> fra3x6(new FraRateHelper(
            Handle<Quote>(fra3x6Rate),
            3, 6, fixingDays, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x9(new FraRateHelper(
            Handle<Quote>(fra6x9Rate),
            6, 9, fixingDays, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x12(new FraRateHelper(
            Handle<Quote>(fra6x12Rate),
            6, 12, fixingDays, calendar, ModifiedFollowing,
            depositDayCounter));


        // setup futures
        Rate convexityAdjustment = 0.0;
        Integer futMonths = 3;
        Date imm = Date::nextIMMdate(settlementDate);
        boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));


        // setup swaps
        Frequency swFixedLegFrequency = Annual;
        BusinessDayConvention swFixedLegConvention = Unadjusted;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
        boost::shared_ptr<Xibor> swFloatingLegIndex(new Euribor6M);

        boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
            Handle<Quote>(s2yRate),
            2*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
            Handle<Quote>(s3yRate),
            3*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
            Handle<Quote>(s5yRate),
            5*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(
            Handle<Quote>(s10yRate),
            10*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(
            Handle<Quote>(s15yRate),
            15*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        DayCounter termStructureDayCounter =
            ActualActual(ActualActual::ISDA);


        double tolerance = 1.0e-15;

        // A depo-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoSwapInstruments;
        depoSwapInstruments.push_back(d1w);
        depoSwapInstruments.push_back(d1m);
        depoSwapInstruments.push_back(d3m);
        depoSwapInstruments.push_back(d6m);
        depoSwapInstruments.push_back(d9m);
        depoSwapInstruments.push_back(d1y);
        depoSwapInstruments.push_back(s2y);
        depoSwapInstruments.push_back(s3y);
        depoSwapInstruments.push_back(s5y);
        depoSwapInstruments.push_back(s10y);
        depoSwapInstruments.push_back(s15y);
        boost::shared_ptr<YieldTermStructure> depoSwapTermStructure(new
            PiecewiseFlatForward(settlementDate, depoSwapInstruments,
                                 termStructureDayCounter, tolerance));


        // A depo-futures-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments;
        depoFutSwapInstruments.push_back(d1w);
        depoFutSwapInstruments.push_back(d1m);
        depoFutSwapInstruments.push_back(fut1);
        depoFutSwapInstruments.push_back(fut2);
        depoFutSwapInstruments.push_back(fut3);
        depoFutSwapInstruments.push_back(fut4);
        depoFutSwapInstruments.push_back(fut5);
        depoFutSwapInstruments.push_back(fut6);
        depoFutSwapInstruments.push_back(fut7);
        depoFutSwapInstruments.push_back(fut8);
        depoFutSwapInstruments.push_back(s3y);
        depoFutSwapInstruments.push_back(s5y);
        depoFutSwapInstruments.push_back(s10y);
        depoFutSwapInstruments.push_back(s15y);
        boost::shared_ptr<YieldTermStructure> depoFutSwapTermStructure(new
            PiecewiseFlatForward(settlementDate, depoFutSwapInstruments,
                                 termStructureDayCounter, tolerance));


        // A depo-FRA-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoFRASwapInstruments;
        depoFRASwapInstruments.push_back(d1w);
        depoFRASwapInstruments.push_back(d1m);
        depoFRASwapInstruments.push_back(d3m);
        depoFRASwapInstruments.push_back(fra3x6);
        depoFRASwapInstruments.push_back(fra6x9);
        depoFRASwapInstruments.push_back(fra6x12);
        depoFRASwapInstruments.push_back(s2y);
        depoFRASwapInstruments.push_back(s3y);
        depoFRASwapInstruments.push_back(s5y);
        depoFRASwapInstruments.push_back(s10y);
        depoFRASwapInstruments.push_back(s15y);
        boost::shared_ptr<YieldTermStructure> depoFRASwapTermStructure(new
            PiecewiseFlatForward(settlementDate, depoFRASwapInstruments,
                                 termStructureDayCounter, tolerance));


        // Term structures that will be used for pricing:
		discountingTermStructure.linkTo(depoFRASwapTermStructure);
		
		}

		const Handle<YieldTermStructure>& yieldTermStructure = discountingTermStructure;
		const Date referenceDate = yieldTermStructure->referenceDate();

		const std::string familyName("");
        const Integer years(10);
        const Integer settlementDays(2);
		const Currency currency = EURCurrency();
        const Calendar calendar = TARGET();
        const Frequency fixedLegFrequency(Annual);
		const Period tenor(6, Months);
        const BusinessDayConvention convention(Following);
        const DayCounter dayCounter = Actual360();
        const boost::shared_ptr<Xibor> iborIndex(new Euribor6M(yieldTermStructure));

		const boost::shared_ptr<SwapIndex> index(
			new SwapIndex(
			familyName, 
			years, 
			settlementDays, 
			currency, 
			calendar, 
			fixedLegFrequency, 
			convention, 
			dayCounter, 
			iborIndex)
			);
		const Date paymentDate(referenceDate+365);
		const Date startDate(referenceDate+180);
		const Date endDate(startDate+365);

		const Real gearing(1);
		const Real spread(0);
		const Integer fixingDays(0);


		boost::shared_ptr<VanillaCMSCouponPricer> numericalPricer(new ConundrumPricerByNumericalIntegration);
		boost::shared_ptr<VanillaCMSCouponPricer> analyticPricer(new ConundrumPricerByBlack);

		std::vector<Date> exerciseDates(2);
		exerciseDates[0] = referenceDate;
		exerciseDates[1] = endDate;

        std::vector<Period> lengths(2);
		lengths[0] = Period(1, Years);
		lengths[1] = Period(30, Years);

        const Matrix volatilities(2,2, .25);

		const Handle<SwaptionVolatilityStructure> swaptionVolatilityStructure(
			boost::shared_ptr<SwaptionVolatilityStructure>(new SwaptionVolatilityMatrix(referenceDate, exerciseDates, lengths, volatilities, dayCounter))
			);

		CMSCoupon coupon1(1, paymentDate, index, startDate, endDate, fixingDays, 
			dayCounter, numericalPricer, gearing, spread, 100, 0);
		coupon1.setSwaptionVolatility(swaptionVolatilityStructure);

		CMSCoupon coupon2(1, paymentDate, index, startDate, endDate, fixingDays, 
			dayCounter, analyticPricer, gearing, spread, 100, 0);
		coupon2.setSwaptionVolatility(swaptionVolatilityStructure);

		const double rate1 = coupon1.rate();
		const double rate2 = coupon2.rate();
		const double difference =  rate2-rate1;

		std::cout << "rate1:\t" << rate1 << "\n";
		std::cout << "rate2:\t" << rate2 << "\n";

		std::cout << "difference:\t" << difference << "\n";

		if (std::fabs(difference) > 1.0e-4) {
                BOOST_ERROR("FAILED\n");
            }
		

    QL_TEST_TEARDOWN
}

test_suite* CmsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CMS tests");
    suite->add(BOOST_TEST_CASE(&CmsTest::testFairRate));
    return suite;
}

