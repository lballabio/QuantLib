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
#include <ql/DayCounters/all.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/CashFlows/conundrumpricer.hpp>
#include <ql/TermStructures/all.hpp>
#include <ql/Volatilities/swaptionvolmatrix.hpp>
#include <ql/Volatilities/swaptionvolcube.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CmsTest)

// global data

Date today_, settlement_;
Calendar calendar_;
BusinessDayConvention fixedConvention_, floatingConvention_;
Frequency fixedFrequency_, floatingFrequency_;
DayCounter fixedDayCount_;
boost::shared_ptr<Xibor> iborIndex_;
Integer settlementDays_;
Handle<YieldTermStructure> termStructure_;
Date referenceDate_;
Date startDate_;
Date paymentDate_;
Date endDate_;
const Real gearing_(1), spread_(0);
const Real infiniteCap_(100), infiniteFloor_(-100);
const Integer years_(30);
boost::shared_ptr<SwapIndex> index_;
std::string familyName_("");

Handle<SwaptionVolatilityStructure> swaptionVolatilityMatrix_;
Handle<SwaptionVolatilityStructure> flatSwaptionVolatilityCube_;
Handle<SwaptionVolatilityStructure> flatSwaptionVolatilityCubeBySabr_;
Handle<SwaptionVolatilityStructure> swaptionVolatilityCubeBySabr_;

const Real volatility_ = .09;

std::vector<GFunctionFactory::ModelOfYieldCurve> modelOfYieldCurves_; 
std::vector<Handle<SwaptionVolatilityStructure> > swaptionVolatilityStructures_;

const Real rateTolerance_ = 2.0e-4;
const Real priceTolerance_ = 2.0e-4;

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

void setup() {

    QL_TEST_BEGIN

    settlementDays_ = 2;
    fixedConvention_ = Unadjusted;
    floatingConvention_ = ModifiedFollowing;
    fixedFrequency_ = Semiannual;
    floatingFrequency_ = Semiannual;
    fixedDayCount_ = Thirty360();

    iborIndex_ = boost::shared_ptr<Xibor>(
        new Euribor(Period(12/floatingFrequency_,
        Months),
        termStructure_));

    calendar_ = iborIndex_->calendar();
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_, 0.05, Actual365Fixed()));
    referenceDate_ = termStructure_->referenceDate();
    startDate_ = (referenceDate_+2*3600);
    paymentDate_ = (startDate_+365);
    endDate_ = (paymentDate_);
    index_ = boost::shared_ptr<SwapIndex>(
        new SwapIndex(
        familyName_,
        years_,
        settlementDays_,
        iborIndex_->currency(),
        calendar_,
        fixedFrequency_,
        fixedConvention_,
        iborIndex_->dayCounter(),
        iborIndex_)
        );


    // Volatility
    std::vector<Date> exerciseDates(2);
    exerciseDates[0] = referenceDate_+30;
    exerciseDates[1] = endDate_;

    std::vector<Period> lengths(2);
    lengths[0] = Period(1, Years);
    lengths[1] = Period(30, Years);

    const Matrix volatilities(2, 2, volatility_);

    swaptionVolatilityMatrix_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(referenceDate_, exerciseDates, lengths,
                                     volatilities, 
                                     iborIndex_->dayCounter())));

    std::vector<Rate> strikeSpreads;

    strikeSpreads.push_back(-.02);
    strikeSpreads.push_back(-.01);
    strikeSpreads.push_back(-.005);
    strikeSpreads.push_back(-.0025);
    strikeSpreads.push_back(.0);
    strikeSpreads.push_back(.0025);
    strikeSpreads.push_back(.005);
    strikeSpreads.push_back(.01);
    strikeSpreads.push_back(.02);


    const Matrix nullVolSpreads(lengths.size()*lengths.size(),
        strikeSpreads.size(), 0.0);

    flatSwaptionVolatilityCube_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
        SwaptionVolatilityCube(
            swaptionVolatilityMatrix_, 
            lengths, 
            lengths,
            strikeSpreads,
            nullVolSpreads,
            calendar_,
            2,
            fixedFrequency_,
            fixedConvention_,
            iborIndex_->dayCounter(),
            iborIndex_,
            1,
            iborIndex_
           )));
    
    flatSwaptionVolatilityCubeBySabr_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
        SwaptionVolatilityCubeBySabr(
            swaptionVolatilityMatrix_, 
            lengths, 
            lengths,
            strikeSpreads,
            nullVolSpreads,
            calendar_,
            2,
            fixedFrequency_,
            fixedConvention_,
            iborIndex_->dayCounter(),
            iborIndex_,
            1,
            iborIndex_,
            1.,
            1e-6
            )));

    Matrix volSpreads(lengths.size()*lengths.size(),
        strikeSpreads.size(), 0.0);

    for(Size i=0; i<strikeSpreads.size(); i++) {
        const double x = strikeSpreads[i];
        const double vs = 10*x*x;
        volSpreads[0][i] = vs;
        volSpreads[1][i] = vs;
    }

    swaptionVolatilityCubeBySabr_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
        SwaptionVolatilityCubeBySabr(
            swaptionVolatilityMatrix_, 
            lengths, 
            lengths,
            strikeSpreads,
            volSpreads,
            calendar_,
            2,
            fixedFrequency_,
            fixedConvention_,
            iborIndex_->dayCounter(),
            iborIndex_,
            1,
            iborIndex_,
            .7,
            1E-3
            )));

    swaptionVolatilityStructures_.push_back(swaptionVolatilityMatrix_);
    //swaptionVolatilityStructures_.push_back(flatSwaptionVolatilityCube_);
    swaptionVolatilityStructures_.push_back(flatSwaptionVolatilityCubeBySabr_);
    swaptionVolatilityStructures_.push_back(swaptionVolatilityCubeBySabr_);

    {
		modelOfYieldCurves_.push_back(GFunctionFactory::standard);
		modelOfYieldCurves_.push_back(GFunctionFactory::exactYield);
		modelOfYieldCurves_.push_back(GFunctionFactory::parallelShifts);
		modelOfYieldCurves_.push_back(GFunctionFactory::nonParallelShifts);
	}
    QL_TEST_TEARDOWN
}


QL_END_TEST_LOCALS(CmsTest)

void CmsTest::testFairRate()  {

    BOOST_MESSAGE(
              "Testing fair-rate calculation for constant-maturity coupons...");

    QL_TEST_BEGIN
    
    for(Size h=0; h<modelOfYieldCurves_.size(); h++) {

    boost::shared_ptr<VanillaCMSCouponPricer> numericalPricer(
        new ConundrumPricerByNumericalIntegration(
            modelOfYieldCurves_[h], 0, 1));
    boost::shared_ptr<VanillaCMSCouponPricer> analyticPricer(
        new ConundrumPricerByBlack(
            modelOfYieldCurves_[h])
            );

    //Coupons
    CMSCoupon coupon1(1, 
        paymentDate_, index_, startDate_, endDate_, settlementDays_,
        iborIndex_->dayCounter(), 
        numericalPricer, gearing_, spread_, infiniteCap_, infiniteFloor_);
    coupon1.setSwaptionVolatility(swaptionVolatilityMatrix_);

    CMSCoupon coupon2(1, 
        paymentDate_, index_, startDate_, endDate_, settlementDays_,
        iborIndex_->dayCounter(), 
        analyticPricer, gearing_, spread_, infiniteCap_, infiniteFloor_);
    coupon2.setSwaptionVolatility(swaptionVolatilityMatrix_);

    //Computation
    const double rate1 = coupon1.rate();
    const double rate2 = coupon2.rate();
    const double difference =  rate2-rate1;

    if (std::fabs(difference) > rateTolerance_) {
            BOOST_ERROR("\n" <<
                "startDate:\t" << startDate_ << "\n"
                        "rate1:\t" << io::rate(rate1) << "\n"
                        "rate2:\t" << io::rate(rate2) << "\n"
                        "difference:\t" << io::rate(difference) << "\n"
                        "tolerance: \t" << io::rate(rateTolerance_));
    }
    }
    QL_TEST_TEARDOWN
}

void CmsTest::testParity() {

    BOOST_MESSAGE("Testing put-call parity for constant-maturity coupons...");

    QL_TEST_BEGIN

    int priceIndex = 1;

    for(Size modelOfYieldCurveIndex=0; 
        modelOfYieldCurveIndex<modelOfYieldCurves_.size();
        modelOfYieldCurveIndex++) {

        std::vector<boost::shared_ptr<VanillaCMSCouponPricer> > pricers;
        {
            boost::shared_ptr<VanillaCMSCouponPricer> analyticPricer(
                new ConundrumPricerByBlack(modelOfYieldCurves_[
                    modelOfYieldCurveIndex]));
            pricers.push_back(analyticPricer);

            boost::shared_ptr<VanillaCMSCouponPricer> numericalPricer(
                new ConundrumPricerByNumericalIntegration(
                modelOfYieldCurves_[modelOfYieldCurveIndex],
                0, 1));
            pricers.push_back(numericalPricer);
        }


        for(Size volStructureIndex = 0; 
            volStructureIndex < swaptionVolatilityStructures_.size(); 
        volStructureIndex++) {

            for(Size pricerIndex=0; pricerIndex<pricers.size(); pricerIndex++) {

                CMSCoupon swaplet(1, 
                    paymentDate_, index_, startDate_, endDate_, settlementDays_,
                    iborIndex_->dayCounter(), 
                    pricers[pricerIndex], gearing_, spread_, infiniteCap_, infiniteFloor_);
                swaplet.setSwaptionVolatility(
                    swaptionVolatilityStructures_[volStructureIndex]
                );

                Real strike = .02;

                for(Size strikeIndex=0; strikeIndex<10; strikeIndex++) {

                BOOST_MESSAGE("(" 
                    << volStructureIndex << ", " 
                    << pricerIndex << ", " 
                    << modelOfYieldCurveIndex << ", " 
                    << strikeIndex
                    << ")\t" << "testing " << priceIndex 
                    << "-th cms swaplet, caplet and floolet with strike " 
                    << io::rate(strike) << "\n");

                    strike += .005;
                    CMSCoupon caplet(1, 
                        paymentDate_, index_, startDate_, endDate_, settlementDays_,
                        iborIndex_->dayCounter(), 
                        pricers[pricerIndex], gearing_, spread_, strike, infiniteFloor_);
                    caplet.setSwaptionVolatility(
                        swaptionVolatilityStructures_[volStructureIndex]);

                    CMSCoupon floorlet(1, 
                        paymentDate_, index_, startDate_, endDate_, settlementDays_,
                        iborIndex_->dayCounter(), 
                        pricers[pricerIndex], gearing_, spread_, infiniteCap_, strike);
                    floorlet.setSwaptionVolatility(
                        swaptionVolatilityStructures_[volStructureIndex]);

                    //Computation
                    const double price1 = swaplet.price(termStructure_)
                        + swaplet.accrualPeriod()*strike* termStructure_->discount(paymentDate_);
                    const double price2 = floorlet.price(termStructure_)
                        + caplet.price(termStructure_);

                    priceIndex++;

                    const double difference =  price1-price2;

                    if (std::fabs(difference) > priceTolerance_) {
                            BOOST_ERROR("\n" <<
                                "startDate:\t" << startDate_ << "\n"
                                "strike:\t" << strike << "\n"
                                "price1:\t" << io::rate(price1) << "\n"
                                "price2:\t" << io::rate(price2) << "\n"
                                "difference:\t" << io::rate(difference) << "\n"
                                "tolerance: \t" << io::rate(priceTolerance_));
                    }
                }
            }
        }
    }
    QL_TEST_TEARDOWN
}


void CmsTest::testCmsSwap() {

    BOOST_MESSAGE("Testing constant-maturity swaps...");

    QL_TEST_BEGIN

     {
        modelOfYieldCurves_.clear();
	//	modelOfYieldCurves_.push_back(GFunctionFactory::standard);
	//	modelOfYieldCurves_.push_back(GFunctionFactory::exactYield);
		modelOfYieldCurves_.push_back(GFunctionFactory::parallelShifts);
		modelOfYieldCurves_.push_back(GFunctionFactory::nonParallelShifts);
	}

    std::vector<Size> swapLengths;
    swapLengths.push_back(1);
    swapLengths.push_back(5);
    swapLengths.push_back(6);
    swapLengths.push_back(10);
    swapLengths.push_back(15);
    swapLengths.push_back(20);
    //swapLengths.push_back(30);

    int priceIndex = 1;

    for(Size swapLengthIndex = 0; swapLengthIndex<swapLengths.size();
        swapLengthIndex++) {

    const int n = swapLengths[swapLengthIndex];

    std::vector<Real> meanReversions(n, 0);
    std::vector<Real> fixedNominals(n, 1);
    std::vector<Real> floatingNominals(n, 1);
    std::vector<Real> caps(n, infiniteCap_);
    std::vector<Real> floors(n, infiniteFloor_);
    std::vector<Real> fractions(n, gearing_);
    std::vector<Real> baseRate(n, 0);

    
    
    for(Size volStructureIndex = 0; 
        volStructureIndex < swaptionVolatilityStructures_.size(); 
        volStructureIndex++) {

        for(Size modelOfYieldCurveIndex=0; 
            modelOfYieldCurveIndex<modelOfYieldCurves_.size(); 
            modelOfYieldCurveIndex++) {
     
            std::vector<boost::shared_ptr<VanillaCMSCouponPricer> > pricers;
            {
                boost::shared_ptr<VanillaCMSCouponPricer> analyticPricer(
                    new ConundrumPricerByBlack(
                    modelOfYieldCurves_[modelOfYieldCurveIndex]));
                pricers.push_back(analyticPricer);

                boost::shared_ptr<VanillaCMSCouponPricer> numericalPricer(
                    new ConundrumPricerByNumericalIntegration(
                    modelOfYieldCurves_[modelOfYieldCurveIndex],
                    0, 1));
                pricers.push_back(numericalPricer);
            }

            Date startDate = today_;
            startDate += Period(10, Days);
            Date maturityDate = startDate;
            maturityDate += Period(n, Years);

            Schedule fixedSchedule(calendar_,startDate, maturityDate,
                fixedFrequency_,fixedConvention_);
            Schedule floatingSchedule(calendar_,startDate, maturityDate,
                floatingFrequency_,floatingConvention_);
            
            std::vector<Real> prices;
            for(Size pricerIndex=0; pricerIndex<pricers.size(); 
                pricerIndex++) {

                std::vector<boost::shared_ptr<CashFlow> > cmsLeg = 
                    CMSCouponVector(
                    fixedSchedule,
                    fixedConvention_,
                    fixedNominals,
                    index_,
                    settlementDays_,
                    fixedDayCount_,
                    baseRate,
                    fractions,
                    caps,
                    floors,
                    meanReversions,
                    pricers[pricerIndex],
                    swaptionVolatilityStructures_[volStructureIndex]
                );

                std::vector<boost::shared_ptr<CashFlow> > floatingLeg = 
                    FloatingRateCouponVector(
                    floatingSchedule, 
                    floatingConvention_, 
                    floatingNominals,
                    settlementDays_, 
                    iborIndex_,
                    std::vector<Real>(),
                    std::vector<Spread>(),
                    iborIndex_->dayCounter());

            
                boost::shared_ptr<Swap> swap =  boost::shared_ptr<Swap>(
                    new Swap(termStructure_, cmsLeg, floatingLeg));


                BOOST_MESSAGE("(" 
                    << volStructureIndex << ", " 
                    << pricerIndex << ", " 
                    << modelOfYieldCurveIndex << ", " 
                    << swapLengthIndex << ")\t" 
                    << priceIndex << "-th pricing "
                    << "for cms swap test...\n");
                priceIndex++;

                prices.push_back(swap->NPV());
            }
            const double difference =  prices[0]-prices[1];

            if (std::fabs(difference) > priceTolerance_) {
                BOOST_ERROR("\n" <<
                    "startDate:\t" << startDate << "\n"
                    "maturityDate:\t" << maturityDate << "\n"
                    //"strike:\t" << 0 << "\n"
                    "price analytic:\t" << io::rate(prices[0]) << "\n"
                    "price numerical:\t" << io::rate(prices[1]) << "\n"
                            "difference:\t" << io::rate(difference) << "\n"
                            "tolerance: \t" << io::rate(priceTolerance_));
            }
        }
    }
    }
    QL_TEST_TEARDOWN
}

test_suite* CmsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CMS tests");

    QL_TEST_SETUP
    suite->add(BOOST_TEST_CASE(&CmsTest::testFairRate));
    suite->add(BOOST_TEST_CASE(&CmsTest::testParity));
    suite->add(BOOST_TEST_CASE(&CmsTest::testCmsSwap));
    return suite;
}





//
//
//
//
//
//boost::shared_ptr<YieldTermStructure> CmsTest::getYieldTermStructure() {
//
//         /*********************
//         ***  MARKET DATA  ***
//         *********************/
//
//        Calendar calendar = TARGET();
//        // uncommenting the following line generates an error
//        // calendar = Tokyo();
//        Date settlementDate(22, September, 2004);
//        // must be a business day
//        settlementDate = calendar.adjust(settlementDate);
//
//        Integer fixingDays = 2;
//        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
//        // nothing to do with Date::todaysDate
//        Settings::instance().evaluationDate() = todaysDate;
//
//
//        todaysDate = Settings::instance().evaluationDate();
//
//        // deposits
//        Rate d1wQuote=0.04;
//        Rate d1mQuote=0.04;
//        Rate d3mQuote=0.04;
//        Rate d6mQuote=0.04;
//        Rate d9mQuote=0.04;
//        Rate d1yQuote=0.04;
//        // FRAs
//        Rate fra3x6Quote=0.04;
//        Rate fra6x9Quote=0.04;
//        Rate fra6x12Quote=0.04;
//        // futures
//        Real fut1Quote=96.2875;
//        Real fut2Quote=96.7875;
//        Real fut3Quote=96.9875;
//        Real fut4Quote=96.6875;
//        Real fut5Quote=96.4875;
//        Real fut6Quote=96.3875;
//        Real fut7Quote=96.2875;
//        Real fut8Quote=96.0875;
//        // swaps
//        Rate s2yQuote=0.04;
//        Rate s3yQuote=0.04;
//        Rate s5yQuote=0.04;
//        Rate s10yQuote=0.04;
//        Rate s15yQuote=0.04;
//        Rate s60yQuote=0.04;
//
//        //        // deposits
//  //      Rate d1wQuote=0.0382;
//  //      Rate d1mQuote=0.0372;
//  //      Rate d3mQuote=0.0363;
//  //      Rate d6mQuote=0.0353;
//  //      Rate d9mQuote=0.0348;
//  //      Rate d1yQuote=0.0345;
//  //      // FRAs
//  //      Rate fra3x6Quote=0.037125;
//  //      Rate fra6x9Quote=0.037125;
//  //      Rate fra6x12Quote=0.037125;
//  //      // futures
//  //      Real fut1Quote=96.2875;
//  //      Real fut2Quote=96.7875;
//  //      Real fut3Quote=96.9875;
//  //      Real fut4Quote=96.6875;
//  //      Real fut5Quote=96.4875;
//  //      Real fut6Quote=96.3875;
//  //      Real fut7Quote=96.2875;
//  //      Real fut8Quote=96.0875;
//  //      // swaps
//  //      Rate s2yQuote=0.037125;
//  //      Rate s3yQuote=0.0398;
//  //      Rate s5yQuote=0.0443;
//  //      Rate s10yQuote=0.05165;
//  //      Rate s15yQuote=0.055175;
//        //Rate s60yQuote=0.053175;
//
//
//        /********************
//         ***    QUOTES    ***
//         ********************/
//
//        // SimpleQuote stores a value which can be manually changed;
//        // other Quote subclasses could read the value from a database
//        // or some kind of data feed.
//
//        // deposits
//        boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
//        boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
//        boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
//        boost::shared_ptr<Quote> d6mRate(new SimpleQuote(d6mQuote));
//        boost::shared_ptr<Quote> d9mRate(new SimpleQuote(d9mQuote));
//        boost::shared_ptr<Quote> d1yRate(new SimpleQuote(d1yQuote));
//        // FRAs
//        boost::shared_ptr<Quote> fra3x6Rate(new SimpleQuote(fra3x6Quote));
//        boost::shared_ptr<Quote> fra6x9Rate(new SimpleQuote(fra6x9Quote));
//        boost::shared_ptr<Quote> fra6x12Rate(new SimpleQuote(fra6x12Quote));
//        // futures
//        boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));
//        boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));
//        boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));
//        boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));
//        boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));
//        boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));
//        boost::shared_ptr<Quote> fut7Price(new SimpleQuote(fut7Quote));
//        boost::shared_ptr<Quote> fut8Price(new SimpleQuote(fut8Quote));
//        // swaps
//        boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
//        boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
//        boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
//        boost::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));
//        boost::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));
//        boost::shared_ptr<Quote> s60yRate(new SimpleQuote(s60yQuote));
//
//
//        /*********************
//         ***  RATE HELPERS ***
//         *********************/
//
//        // RateHelpers are built from the above quotes together with
//        // other instrument dependant infos.  Quotes are passed in
//        // relinkable handles which could be relinked to some other
//        // data source later.
//
//        // deposits
//        DayCounter depositDayCounter = Actual360();
//
//        boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(
//            Handle<Quote>(d1wRate),
//            1*Weeks, fixingDays,
//            calendar, ModifiedFollowing, depositDayCounter));
//        boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(
//            Handle<Quote>(d1mRate),
//            1*Months, fixingDays,
//            calendar, ModifiedFollowing, depositDayCounter));
//        boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(
//            Handle<Quote>(d3mRate),
//            3*Months, fixingDays,
//            calendar, ModifiedFollowing, depositDayCounter));
//        boost::shared_ptr<RateHelper> d6m(new DepositRateHelper(
//            Handle<Quote>(d6mRate),
//            6*Months, fixingDays,
//            calendar, ModifiedFollowing, depositDayCounter));
//        boost::shared_ptr<RateHelper> d9m(new DepositRateHelper(
//            Handle<Quote>(d9mRate),
//            9*Months, fixingDays,
//            calendar, ModifiedFollowing, depositDayCounter));
//        boost::shared_ptr<RateHelper> d1y(new DepositRateHelper(
//            Handle<Quote>(d1yRate),
//            1*Years, fixingDays,
//            calendar, ModifiedFollowing, depositDayCounter));
//
//
//        // setup FRAs
//        boost::shared_ptr<RateHelper> fra3x6(new FraRateHelper(
//            Handle<Quote>(fra3x6Rate),
//            3, 6, fixingDays, calendar, ModifiedFollowing,
//            depositDayCounter));
//        boost::shared_ptr<RateHelper> fra6x9(new FraRateHelper(
//            Handle<Quote>(fra6x9Rate),
//            6, 9, fixingDays, calendar, ModifiedFollowing,
//            depositDayCounter));
//        boost::shared_ptr<RateHelper> fra6x12(new FraRateHelper(
//            Handle<Quote>(fra6x12Rate),
//            6, 12, fixingDays, calendar, ModifiedFollowing,
//            depositDayCounter));
//
//
//        // setup futures
//        Rate convexityAdjustment = 0.0;
//        Integer futMonths = 3;
//        Date imm = Date::nextIMMdate(settlementDate);
//        boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//        imm = Date::nextIMMdate(imm+1);
//        boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(
//            Handle<Quote>(fut1Price),
//            imm,
//            futMonths, calendar, ModifiedFollowing,
//            depositDayCounter, convexityAdjustment));
//
//
//        // setup swaps
//        Frequency swFixedLegFrequency = Annual;
//        BusinessDayConvention swFixedLegConvention = Unadjusted;
//        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
//        boost::shared_ptr<Xibor> swFloatingLegIndex(new Euribor6M);
//
//        boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
//            Handle<Quote>(s2yRate),
//            2*Years, fixingDays,
//            calendar, swFixedLegFrequency,
//            swFixedLegConvention, swFixedLegDayCounter,
//            swFloatingLegIndex));
//        boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
//            Handle<Quote>(s3yRate),
//            3*Years, fixingDays,
//            calendar, swFixedLegFrequency,
//            swFixedLegConvention, swFixedLegDayCounter,
//            swFloatingLegIndex));
//        boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
//            Handle<Quote>(s5yRate),
//            5*Years, fixingDays,
//            calendar, swFixedLegFrequency,
//            swFixedLegConvention, swFixedLegDayCounter,
//            swFloatingLegIndex));
//        boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(
//            Handle<Quote>(s10yRate),
//            10*Years, fixingDays,
//            calendar, swFixedLegFrequency,
//            swFixedLegConvention, swFixedLegDayCounter,
//            swFloatingLegIndex));
//        boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(
//            Handle<Quote>(s15yRate),
//            15*Years, fixingDays,
//            calendar, swFixedLegFrequency,
//            swFixedLegConvention, swFixedLegDayCounter,
//            swFloatingLegIndex));
//        boost::shared_ptr<RateHelper> s60y(new SwapRateHelper(
//            Handle<Quote>(s60yRate),
//            60*Years, fixingDays,
//            calendar, swFixedLegFrequency,
//            swFixedLegConvention, swFixedLegDayCounter,
//            swFloatingLegIndex));
//
//
//        /*********************
//         **  CURVE BUILDING **
//         *********************/
//
//        // Any DayCounter would be fine.
//        // ActualActual::ISDA ensures that 30 years is 30.0
//        DayCounter termStructureDayCounter =
//            ActualActual(ActualActual::ISDA);
//
//
//        double tolerance = 1.0e-15;
//
//        // A depo-swap curve
//        std::vector<boost::shared_ptr<RateHelper> > depoSwapInstruments;
//        depoSwapInstruments.push_back(d1w);
//        depoSwapInstruments.push_back(d1m);
//        depoSwapInstruments.push_back(d3m);
//        depoSwapInstruments.push_back(d6m);
//        depoSwapInstruments.push_back(d9m);
//        depoSwapInstruments.push_back(d1y);
//        depoSwapInstruments.push_back(s2y);
//        depoSwapInstruments.push_back(s3y);
//        depoSwapInstruments.push_back(s5y);
//        depoSwapInstruments.push_back(s10y);
//        depoSwapInstruments.push_back(s15y);
//        depoSwapInstruments.push_back(s60y);
//        boost::shared_ptr<YieldTermStructure> depoSwapTermStructure(new
//            PiecewiseFlatForward(settlementDate, depoSwapInstruments,
//                                 termStructureDayCounter, tolerance));
//
//
//        // A depo-futures-swap curve
//        std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments;
//        depoFutSwapInstruments.push_back(d1w);
//        depoFutSwapInstruments.push_back(d1m);
//        depoFutSwapInstruments.push_back(fut1);
//        depoFutSwapInstruments.push_back(fut2);
//        depoFutSwapInstruments.push_back(fut3);
//        depoFutSwapInstruments.push_back(fut4);
//        depoFutSwapInstruments.push_back(fut5);
//        depoFutSwapInstruments.push_back(fut6);
//        depoFutSwapInstruments.push_back(fut7);
//        depoFutSwapInstruments.push_back(fut8);
//        depoFutSwapInstruments.push_back(s3y);
//        depoFutSwapInstruments.push_back(s5y);
//        depoFutSwapInstruments.push_back(s10y);
//        depoFutSwapInstruments.push_back(s15y);
//        depoFutSwapInstruments.push_back(s60y);
//        boost::shared_ptr<YieldTermStructure> depoFutSwapTermStructure(new
//            PiecewiseFlatForward(settlementDate, depoFutSwapInstruments,
//                                 termStructureDayCounter, tolerance));
//
//
//        // A depo-FRA-swap curve
//        std::vector<boost::shared_ptr<RateHelper> > depoFRASwapInstruments;
//        depoFRASwapInstruments.push_back(d1w);
//        depoFRASwapInstruments.push_back(d1m);
//        depoFRASwapInstruments.push_back(d3m);
//        depoFRASwapInstruments.push_back(fra3x6);
//        depoFRASwapInstruments.push_back(fra6x9);
//        depoFRASwapInstruments.push_back(fra6x12);
//        depoFRASwapInstruments.push_back(s2y);
//        depoFRASwapInstruments.push_back(s3y);
//        depoFRASwapInstruments.push_back(s5y);
//        depoFRASwapInstruments.push_back(s10y);
//        depoFRASwapInstruments.push_back(s15y);
//        depoFRASwapInstruments.push_back(s60y);
//        boost::shared_ptr<YieldTermStructure> depoFRASwapTermStructure(new
//            PiecewiseFlatForward(settlementDate, depoFRASwapInstruments,
//                                 termStructureDayCounter, tolerance));
//        return depoFRASwapTermStructure;
//}
