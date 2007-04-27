/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti

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

#include "digitalcoupon.hpp"
#include "utilities.hpp"
#include <ql/indexes/euribor.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/volatilities/capletconstantvol.hpp>
#include <ql/pricingengines/blackformula.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(DigitalCouponTest)

// global data

Date today_, settlement_;
Real nominal_;
Calendar calendar_;
boost::shared_ptr<IborIndex> index_;
Natural fixingDays_;
RelinkableHandle<YieldTermStructure> termStructure_;

void setup() {
    fixingDays_ = 2;
    nominal_ = 1000000.0;
    index_ = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    calendar_ = index_->calendar();
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlement_ = calendar_.advance(today_,fixingDays_,Days);
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365Fixed()));  // by default: Continuous and annual
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(DigitalCouponTest)

void DigitalCouponTest::testCashOrNothingDeepInTheMoney() {

    BOOST_MESSAGE("Testing European deep in-the-money cash-or-nothing digital coupon ...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Real gearing = 1.0;
    Real spread = 0.0;

    Volatility capletVolatility = 0.0001;
    RelinkableHandle<CapletVolatilityStructure> volatility;
    volatility.linkTo(boost::shared_ptr<CapletVolatilityStructure>(new
                        CapletConstantVolatility(today_, capletVolatility, Actual360())));
            
    for (Size k = 0; k<10; k++) {
        Date startDate = calendar_.advance(settlement_,(k+1)*Years);
        Date endDate = calendar_.advance(settlement_,(k+2)*Years);
        Rate nullstrike = Null<Rate>();
        Rate cashRate = 0.01;
        Real gap = 1e-4;
        Date paymentDate = endDate;

        boost::shared_ptr<FloatingRateCoupon> underlying(new IborCoupon(paymentDate,
                                                                        nominal_,
                                                                        startDate,
                                                                        endDate,
                                                                        fixingDays_,
                                                                        index_,
                                                                        gearing,
                                                                        spread));
        // Capped Digital Coupon
        Rate strike = 0.001;
        DigitalCoupon digitalCappedCoupon(underlying, strike, nullstrike, cashRate, gap);    
        boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(volatility));
        digitalCappedCoupon.setPricer(pricer);
        
        // Check price vs decomposition used in digital coupon class implementation
        CappedFlooredIborCoupon cappedIborCoupon_d(paymentDate,
                                                   nominal_,
                                                   startDate,
                                                   endDate,
                                                   fixingDays_,
                                                   index_,
                                                   gearing,
                                                   spread,
                                                   strike - gap);
        cappedIborCoupon_d.setPricer(pricer);
        CappedFlooredIborCoupon cappedIborCoupon_u(paymentDate,
                                                   nominal_,
                                                   startDate,
                                                   endDate,
                                                   fixingDays_,
                                                   index_,
                                                   gearing,
                                                   spread,
                                                   strike + gap);
        cappedIborCoupon_u.setPricer(pricer);
            
        Time accrualPeriod = Actual360().yearFraction(startDate, endDate);
        Real discount = termStructure_->discount(endDate);

        Real targetPrice = underlying->price(termStructure_) -
                           cashRate * nominal_ * accrualPeriod * discount;
        Real digitalPrice = digitalCappedCoupon.price(termStructure_);
        
        Real error = std::fabs(digitalPrice- targetPrice)/targetPrice;
        Real tolerance = 1e-10;
        if (error>tolerance) {
            BOOST_ERROR("\nDigital Capped Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital coupon price "  << digitalPrice <<
                        "\nTarget price  " << targetPrice <<
                        "\nError " << io::rate(error) );
        }
            
        // Check digital option price
        Real targetOptionPrice = cashRate * nominal_ * accrualPeriod * discount;
        Real replicationOptionPrice = digitalCappedCoupon.optionRate()* nominal_ * accrualPeriod * discount;
        error = std::abs(replicationOptionPrice-targetOptionPrice);
        Real optionTolerance = 1e-08;
        if (error>optionTolerance) {
            BOOST_ERROR("\nDigital Capped Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital option price by replication "  << replicationOptionPrice <<
                        "\nDigital option target price " << targetOptionPrice <<
                        "\nError " << error );
        }
            
        // Floored Digital Coupon
        strike = 0.99;
        DigitalCoupon digitalFlooredCoupon(underlying, nullstrike, strike, cashRate, gap);   
        digitalFlooredCoupon.setPricer(pricer);

        // Check price vs decomposition used in digital coupon class implementation
        CappedFlooredIborCoupon flooredIborCoupon_d(paymentDate,
                                                    nominal_,
                                                    startDate,
                                                    endDate,
                                                    fixingDays_,
                                                    index_,
                                                    gearing,
                                                    spread,
                                                    nullstrike,
                                                    strike - gap);
        flooredIborCoupon_d.setPricer(pricer);
        CappedFlooredIborCoupon flooredIborCoupon_u(paymentDate,
                                                    nominal_,
                                                    startDate,
                                                    endDate,
                                                    fixingDays_,
                                                    index_,
                                                    gearing,
                                                    spread,
                                                    nullstrike,
                                                    strike + gap);
        flooredIborCoupon_u.setPricer(pricer);

        targetPrice = underlying->price(termStructure_) +
                      cashRate * nominal_ * accrualPeriod * discount;
        digitalPrice = digitalFlooredCoupon.price(termStructure_);
        
        error = std::fabs(digitalPrice - targetPrice)/targetPrice;
        if (error>tolerance) {
            BOOST_ERROR("\nDigital Floored Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital coupon price "  << digitalPrice <<
                        "\nTarget price  " << targetPrice <<
                        "\nError " << io::rate(error) );
        }

        // Check digital option
        targetOptionPrice = cashRate * nominal_ * accrualPeriod * discount;
        replicationOptionPrice = digitalFlooredCoupon.optionRate()* nominal_ * accrualPeriod * discount;
        error = std::abs(replicationOptionPrice-targetOptionPrice);
        if (error>optionTolerance) {
            BOOST_ERROR("\nDigital Floored Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital option price by replication "  << replicationOptionPrice <<
                        "\nDigital option target price " << targetOptionPrice <<
                        "\nError " << error );
        }
    }        
    QL_TEST_TEARDOWN
}

void DigitalCouponTest::testCashOrNothingDeepOutTheMoney() {

    BOOST_MESSAGE("Testing European deep out-the-money cash-or-nothing digital coupon ...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Real gearing = 1.0;
    Real spread = 0.0;

    Volatility capletVolatility = 0.0001;
    RelinkableHandle<CapletVolatilityStructure> volatility;
    volatility.linkTo(boost::shared_ptr<CapletVolatilityStructure>(new
                        CapletConstantVolatility(today_, capletVolatility, Actual360())));
            
    for (Size k = 0; k<10; k++) {
        Date startDate = calendar_.advance(settlement_,(k+1)*Years);
        Date endDate = calendar_.advance(settlement_,(k+2)*Years);
        Rate nullstrike = Null<Rate>();
        Rate cashRate = 0.01;
        Real gap = 1e-4;
        Date paymentDate = endDate;

        boost::shared_ptr<FloatingRateCoupon> underlying(new IborCoupon(paymentDate,
                                                                        nominal_,
                                                                        startDate,
                                                                        endDate,
                                                                        fixingDays_,
                                                                        index_,
                                                                        gearing,
                                                                        spread));
        // Capped Digital Coupon
        Rate strike = 0.99;
        DigitalCoupon digitalCappedCoupon(underlying, strike, nullstrike, cashRate, gap);    
        boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(volatility));
        digitalCappedCoupon.setPricer(pricer);
        
        // Check price vs decomposition used in digital coupon class implementation
        CappedFlooredIborCoupon cappedIborCoupon_d(paymentDate,
                                                   nominal_,
                                                   startDate,
                                                   endDate,
                                                   fixingDays_,
                                                   index_,
                                                   gearing,
                                                   spread,
                                                   strike - gap);
        cappedIborCoupon_d.setPricer(pricer);
        CappedFlooredIborCoupon cappedIborCoupon_u(paymentDate,
                                                   nominal_,
                                                   startDate,
                                                   endDate,
                                                   fixingDays_,
                                                   index_,
                                                   gearing,
                                                   spread,
                                                   strike + gap);
        cappedIborCoupon_u.setPricer(pricer);
            
        Time accrualPeriod = Actual360().yearFraction(startDate, endDate);
        Real discount = termStructure_->discount(endDate);

        Real targetPrice = underlying->price(termStructure_);
        Real digitalPrice = digitalCappedCoupon.price(termStructure_);
        
        Real error = std::fabs(digitalPrice- targetPrice)/targetPrice;
        Real tolerance = 1e-10;
        if (error>tolerance) {
            BOOST_ERROR("\nDigital Capped Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital coupon price "  << digitalPrice <<
                        "\nTarget price  " << targetPrice <<
                        "\nError " << io::rate(error) );
        }
            
        // Check digital option price
        Real targetOptionPrice = 0.;
        Real replicationOptionPrice = digitalCappedCoupon.optionRate()* nominal_ * accrualPeriod * discount;
        error = std::abs(replicationOptionPrice-targetOptionPrice);
        Real optionTolerance = 1e-08;
        if (error>optionTolerance) {
            BOOST_ERROR("\nDigital Capped Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital option price by replication "  << replicationOptionPrice <<
                        "\nDigital option target price " << targetOptionPrice <<
                        "\nError " << error );
        }
            
        // Floored Digital Coupon
        strike = 0.01;
        DigitalCoupon digitalFlooredCoupon(underlying, nullstrike, strike, cashRate, gap);   
        digitalFlooredCoupon.setPricer(pricer);

        // Check price vs decomposition used in digital coupon class implementation
        CappedFlooredIborCoupon flooredIborCoupon_d(paymentDate,
                                                    nominal_,
                                                    startDate,
                                                    endDate,
                                                    fixingDays_,
                                                    index_,
                                                    gearing,
                                                    spread,
                                                    nullstrike,
                                                    strike - gap);
        flooredIborCoupon_d.setPricer(pricer);
        CappedFlooredIborCoupon flooredIborCoupon_u(paymentDate,
                                                    nominal_,
                                                    startDate,
                                                    endDate,
                                                    fixingDays_,
                                                    index_,
                                                    gearing,
                                                    spread,
                                                    nullstrike,
                                                    strike + gap);
        flooredIborCoupon_u.setPricer(pricer);

        targetPrice = underlying->price(termStructure_);
        digitalPrice = digitalFlooredCoupon.price(termStructure_);
        
        error = std::fabs(digitalPrice - targetPrice)/targetPrice;
        if (error>tolerance) {
            BOOST_ERROR("\nDigital Floored Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital coupon price "  << digitalPrice <<
                        "\nTarget price  " << targetPrice <<
                        "\nError " << io::rate(error) );
        }

        // Check digital option
        targetOptionPrice = 0.0;
        replicationOptionPrice = digitalFlooredCoupon.optionRate()* nominal_ * accrualPeriod * discount;
        error = std::abs(replicationOptionPrice-targetOptionPrice);
        if (error>optionTolerance) {
            BOOST_ERROR("\nDigital Floored Coupon:" << 
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise date = " << k+1 << " years" <<
                        "\nDigital option price by replication "  << replicationOptionPrice <<
                        "\nDigital option target price " << targetOptionPrice <<
                        "\nError " << error );
        }
    }        
    QL_TEST_TEARDOWN
}

void DigitalCouponTest::testCashOrNothing() {

    BOOST_MESSAGE("Testing European cash-or-nothing digital coupon ...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Volatility vols[] = { 0.05, 0.15, 0.30 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };

    Real gearing = 1.0;
    Real spread = 0.0;

    for (Size i = 0; i< LENGTH(vols); i++) {
            Volatility capletVolatility = vols[i];
            RelinkableHandle<CapletVolatilityStructure> volatility;
            volatility.linkTo(boost::shared_ptr<CapletVolatilityStructure>(new
                                CapletConstantVolatility(today_, capletVolatility, Actual360())));
        for (Size j = 0; j< LENGTH(strikes); j++) {
            Rate strike = strikes[j];
            for (Size k = 0; k<10; k++) {
                Date startDate = calendar_.advance(settlement_,(k+1)*Years);
                Date endDate = calendar_.advance(settlement_,(k+2)*Years);
                Rate nullstrike = Null<Rate>();
                Rate cashRate = 0.01;
                Real gap = 1e-8;
                Date paymentDate = endDate;

                boost::shared_ptr<FloatingRateCoupon> underlying(new IborCoupon(paymentDate,
                                                                                nominal_,
                                                                                startDate,
                                                                                endDate,
                                                                                fixingDays_,
                                                                                index_,
                                                                                gearing,
                                                                                spread));
                // Capped Digital Coupon
                DigitalCoupon digitalCappedCoupon(underlying, strike, nullstrike, cashRate, gap);    
                boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(volatility));
                digitalCappedCoupon.setPricer(pricer);
                
                // Check price vs decomposition used in digital coupon class implementation
                CappedFlooredIborCoupon cappedIborCoupon_d(paymentDate,
                                                           nominal_,
                                                           startDate,
                                                           endDate,
                                                           fixingDays_,
                                                           index_,
                                                           gearing,
                                                           spread,
                                                           strike - gap);
                cappedIborCoupon_d.setPricer(pricer);
                CappedFlooredIborCoupon cappedIborCoupon_u(paymentDate,
                                                           nominal_,
                                                           startDate,
                                                           endDate,
                                                           fixingDays_,
                                                           index_,
                                                           gearing,
                                                           spread,
                                                           strike + gap);
                cappedIborCoupon_u.setPricer(pricer);

                Real decompositionPrice = underlying->price(termStructure_) -
                                          cashRate * (cappedIborCoupon_u.price(termStructure_) -
                                                      cappedIborCoupon_d.price(termStructure_) )
                                         /(2.0*gap);
                Real error = std::fabs(digitalCappedCoupon.price(termStructure_)- decompositionPrice)/
                             decompositionPrice;
                Real tolerance = 1e-8;
                if (error>tolerance) {
                    BOOST_ERROR("\nDigital Capped Coupon:" << 
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise date = " << k+1 << " years" <<
                                "\nDigital coupon price "  << digitalCappedCoupon.price(termStructure_) <<
                                "\nDecomposition price  " << decompositionPrice <<
                                "\nError " << io::rate(error) );
                }
                
                // Check digital option price vs N(d2) price
                Date exDate = calendar_.advance(startDate,-2*Days);
                Real forward = underlying->rate();
                Time ttm = Actual360().yearFraction(today_, exDate);
                Time accr = Actual360().yearFraction(startDate, endDate);
                Real discount = termStructure_->discount(endDate);

                Real ITM = blackFormulaCashItmProbability(Option::Call,
                                                          (strike-spread)/gearing,
                                                          forward,
                                                          std::sqrt(ttm) * capletVolatility);
                Real nd2OptionPrice = ITM * nominal_ * accr * discount * cashRate;
                Real replicationOptionPrice = digitalCappedCoupon.optionRate()* nominal_ * accr * discount;
                error = std::abs(replicationOptionPrice-nd2OptionPrice);
                Real optionTolerance = 1e-04;
                if (error>optionTolerance) {
                    BOOST_ERROR("\nDigital Capped Coupon:" << 
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise date = " << k+1 << " years" <<
                                "\nDigital option price by replication "  << replicationOptionPrice <<
                                "\nDigital option price by black formula " << nd2OptionPrice <<
                                "\nError " << error );
                }
                
                // Floored Digital Coupon
                DigitalCoupon digitalFlooredCoupon(underlying, nullstrike, strike, cashRate, gap);   
                digitalFlooredCoupon.setPricer(pricer);

                // Check price vs decomposition used in digital coupon class implementation
                CappedFlooredIborCoupon flooredIborCoupon_d(paymentDate,
                                                            nominal_,
                                                            startDate,
                                                            endDate,
                                                            fixingDays_,
                                                            index_,
                                                            gearing,
                                                            spread,
                                                            nullstrike,
                                                            strike - gap);
                flooredIborCoupon_d.setPricer(pricer);
                CappedFlooredIborCoupon flooredIborCoupon_u(paymentDate,
                                                            nominal_,
                                                            startDate,
                                                            endDate,
                                                            fixingDays_,
                                                            index_,
                                                            gearing,
                                                            spread,
                                                            nullstrike,
                                                            strike + gap);
                flooredIborCoupon_u.setPricer(pricer);

                decompositionPrice = underlying->price(termStructure_) +
                                     cashRate * (flooredIborCoupon_u.price(termStructure_) -
                                                 flooredIborCoupon_d.price(termStructure_) )
                                     /(2.0*gap);

                error = std::fabs(digitalFlooredCoupon.price(termStructure_)- decompositionPrice)/
                        decompositionPrice;
                if (error>tolerance) {
                    BOOST_ERROR("\nDigital Floored Coupon:" << 
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise date = " << k+1 << " years" <<
                                "\nDigital coupon price "  << digitalFlooredCoupon.price(termStructure_) <<
                                "\nDecomposition price  " << decompositionPrice <<
                                "\nError " << io::rate(error) );
                }

                // Check digital option price vs N(d2) price
                ITM = blackFormulaCashItmProbability(Option::Put,
                                                     (strike-spread)/gearing,
                                                     forward,
                                                     std::sqrt(ttm) * capletVolatility);
                nd2OptionPrice = ITM * nominal_ * accr * discount * cashRate;
                replicationOptionPrice = digitalFlooredCoupon.optionRate()* nominal_ * accr * discount;
                error = std::abs(replicationOptionPrice-nd2OptionPrice);
                if (error>optionTolerance) {
                    BOOST_ERROR("\nDigital Floored Coupon:" << 
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise date = " << k+1 << " years" <<
                                "\nDigital option price by replication "  << replicationOptionPrice <<
                                "\nDigital option price by black formula " << nd2OptionPrice <<
                                "\nError " << error );
                }
            }
        }
    }
    QL_TEST_TEARDOWN

}

void DigitalCouponTest::testAssetOrNothing() {

    BOOST_MESSAGE("Testing European asset-or-nothing digital coupon ...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Rate strike = 0.05;
    Rate nullstrike = Null<Rate>();
    Real gap = 1e-8;

    Date startDate = calendar_.advance(settlement_,10*Years);
    Date endDate = calendar_.advance(settlement_,11*Years);
    Date paymentDate = endDate;
    Volatility capletVolatility = 0.20;
    RelinkableHandle<CapletVolatilityStructure> volatility;
    volatility.linkTo(boost::shared_ptr<CapletVolatilityStructure>(new
                        CapletConstantVolatility(today_, capletVolatility, Actual360())));

    boost::shared_ptr<FloatingRateCoupon> underlying(new IborCoupon(paymentDate,
                                                                    nominal_,
                                                                    startDate,
                                                                    endDate,
                                                                    fixingDays_,
                                                                    index_));
    // Capped Digital Coupon
    DigitalCoupon digitalCappedCoupon(underlying, strike, nullstrike, nullstrike, gap);    
    boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(volatility));
    digitalCappedCoupon.setPricer(pricer);

    BOOST_MESSAGE("\nUNDERLYNG IBOR-COUPON: price = " << underlying->price(termStructure_));
    BOOST_MESSAGE("\nDIGITAL CAPPED COUPON: price = " << digitalCappedCoupon.price(termStructure_));
    
    // Check price vs decomposition used in digital coupon class implementation
    CappedFlooredIborCoupon cappedIborCoupon_d(paymentDate,
                                               nominal_,
                                               startDate,
                                               endDate,
                                               fixingDays_,
                                               index_,
                                               1.0,
                                               0.0,
                                               strike - gap);
    cappedIborCoupon_d.setPricer(pricer);
    CappedFlooredIborCoupon cappedIborCoupon_u(paymentDate,
                                               nominal_,
                                               startDate,
                                               endDate,
                                               fixingDays_,
                                               index_,
                                               1.0,
                                               0.0,
                                               strike + gap);
    cappedIborCoupon_u.setPricer(pricer);

    Real callPrice = underlying->price(termStructure_) - 
                     cappedIborCoupon_u.price(termStructure_);
    Real cnPrice = strike * (cappedIborCoupon_u.price(termStructure_) -
                             cappedIborCoupon_d.price(termStructure_) )
                           / (2.0*gap);
    Real decompositionPrice = underlying->price(termStructure_) -
                              (callPrice + cnPrice);

    BOOST_MESSAGE("\nDECOMPOSITION: price = " << decompositionPrice);

    Real error = std::fabs(digitalCappedCoupon.price(termStructure_)- decompositionPrice);
    Real tolerance = 1e-09;
    if (error>tolerance) {
        BOOST_ERROR("digital coupon price "  << digitalCappedCoupon.price(termStructure_) <<
                    " - decomposition price  " << decompositionPrice <<
                    " - error " << error );
    }

    // Check digital option price vs N(d2) price
    Date exDate = calendar_.advance(startDate,-2*Days);
    Real forward = underlying->rate();
    Time ttm = Actual360().yearFraction(today_, exDate);
    Time accr = Actual360().yearFraction(startDate, endDate);
    Real discount = termStructure_->discount(endDate);

    Real ITM = blackFormulaCashItmProbability(Option::Call,
                                              strike,
                                              forward,
                                              std::sqrt(ttm) * capletVolatility);
    Real nd2OptionPrice = ITM * nominal_ * accr * discount * forward;
    Real replicationOptionPrice = digitalCappedCoupon.optionRate()* nominal_ * accr * discount;
    BOOST_MESSAGE("\nDIGITAL OPTION - black formula price: " << nd2OptionPrice <<
                  " replication price " << replicationOptionPrice << 
                  " diff " << replicationOptionPrice-nd2OptionPrice <<
                  " xxx "  << callPrice + cnPrice << 
                  " yyy " << ITM);

    // Floored Digital Coupon
    DigitalCoupon digitalFlooredCoupon(underlying, nullstrike, strike, nullstrike, gap);   
    digitalFlooredCoupon.setPricer(pricer);

    BOOST_MESSAGE("\nDIGITAL FLOORED COUPON: price = " << digitalFlooredCoupon.price(termStructure_));

    // Check price vs decomposition used in digital coupon class implementation
    CappedFlooredIborCoupon flooredIborCoupon_d(paymentDate,
                                                nominal_,
                                                startDate,
                                                endDate,
                                                fixingDays_,
                                                index_,
                                                1.0,
                                                0.0,
                                                nullstrike,
                                                strike - gap);
    flooredIborCoupon_d.setPricer(pricer);
    CappedFlooredIborCoupon flooredIborCoupon_u(paymentDate,
                                                nominal_,
                                                startDate,
                                                endDate,
                                                fixingDays_,
                                                index_,
                                                1.0,
                                                0.0,
                                                nullstrike,
                                                strike + gap);
    flooredIborCoupon_u.setPricer(pricer);

    decompositionPrice = underlying->price(termStructure_) -
                        (underlying->price(termStructure_)  - (callPrice + cnPrice));
                         
    BOOST_MESSAGE("\nDECOMPOSITION: price = "  << decompositionPrice);

    error = std::fabs(digitalFlooredCoupon.price(termStructure_)- decompositionPrice);
    if (error>tolerance) {
        BOOST_ERROR("digital coupon price "  << digitalFlooredCoupon.price(termStructure_) <<
                    " - decomposition price  " << decompositionPrice <<
                    " - error " << error );
    }

    // Check digital option price vs N(d2) price
    ITM = blackFormulaCashItmProbability(Option::Put,
                                         strike,
                                         forward,
                                         std::sqrt(ttm) * capletVolatility);
    nd2OptionPrice = ITM * nominal_ * accr * discount * forward;
    replicationOptionPrice = digitalFlooredCoupon.optionRate()* nominal_ * accr * discount;
    BOOST_MESSAGE("\nDIGITAL OPTION - black formula price: " << nd2OptionPrice <<
                  " replication price " << replicationOptionPrice << 
                  " diff " << replicationOptionPrice-nd2OptionPrice);
   QL_TEST_TEARDOWN

}


test_suite* DigitalCouponTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Digital coupon tests");
    //suite->add(BOOST_TEST_CASE(&DigitalCouponTest::testAssetOrNothing));
    suite->add(BOOST_TEST_CASE(&DigitalCouponTest::testCashOrNothing));
    suite->add(BOOST_TEST_CASE(&DigitalCouponTest::testCashOrNothingDeepInTheMoney));
    suite->add(BOOST_TEST_CASE(&DigitalCouponTest::testCashOrNothingDeepOutTheMoney));
    return suite;
}
