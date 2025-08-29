/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2008 Allen Kuo

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */

/* This example sets up a callable fixed rate bond with a Hull White pricing
   engine and compares to Bloomberg's Hull White price/yield calculations.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/experimental/callablebonds/treecallablebondengine.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actualactual.hpp>

#include <vector>
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace QuantLib;

ext::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             const ext::shared_ptr<Quote>& forward,
             const DayCounter& dc,
             const Compounding& compounding,
             const Frequency& frequency) {
    return ext::make_shared<FlatForward>(today,
                                         Handle<Quote>(forward),
                                         dc,
                                         compounding,
                                         frequency);
}


ext::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             Rate forward,
             const DayCounter& dc,
             const Compounding &compounding,
             const Frequency &frequency) {
    return flatRate(today,
            ext::make_shared<SimpleQuote>(forward),
            dc,
            compounding,
            frequency);
}


int main(int, char* [])
{
    try {


        Date today = Date(16,October,2007);
        Settings::instance().evaluationDate() = today;

        cout <<  endl;
        cout << "Pricing a callable fixed rate bond using" << endl;
        cout << "Hull White model w/ reversion parameter = 0.03" << endl;
        cout << "BAC4.65 09/15/12  ISIN: US06060WBJ36" << endl;
        cout << "roughly five year tenor, ";
        cout << "quarterly coupon and call dates" << endl;
        cout << "reference date is : " << today << endl << endl;

        /* Bloomberg OAS1: "N" model (Hull White)
           varying volatility parameter

           The curve entered into Bloomberg OAS1 is a flat curve,
           at constant yield = 5.5%, semiannual compounding.
           Assume here OAS1 curve uses an ACT/ACT day counter,
           as documented in PFC1 as a "default" in the latter case.
        */

        // set up a flat curve corresponding to Bloomberg flat curve

        Rate bbCurveRate = 0.055;
        DayCounter bbDayCounter = ActualActual(ActualActual::Bond);
        InterestRate bbIR(bbCurveRate,bbDayCounter,Compounded,Semiannual);

        Handle<YieldTermStructure> termStructure(flatRate(today,
                                                          bbIR.rate(),
                                                          bbIR.dayCounter(),
                                                          bbIR.compounding(),
                                                          bbIR.frequency()));

        // set up the call schedule

        CallabilitySchedule callSchedule;
        Real callPrice = 100.;
        Size numberOfCallDates = 24;
        Date callDate = Date(15,September,2006);

        for (Size i=0; i< numberOfCallDates; i++) {
            Calendar nullCalendar = NullCalendar();

            Bond::Price myPrice(callPrice, Bond::Price::Clean);
            callSchedule.push_back(
                ext::make_shared<Callability>(
                                    myPrice,
                                    Callability::Call,
                                    callDate ));
            callDate = nullCalendar.advance(callDate, 3, Months);
        }


        // set up the callable bond

        Date dated = Date(16,September,2004);
        Date issue = dated;
        Date maturity = Date(15,September,2012);
        Natural settlementDays = 3;  // Bloomberg OAS1 settle is Oct 19, 2007
        Calendar bondCalendar = UnitedStates(UnitedStates::GovernmentBond);
        Real coupon = .0465;
        Frequency frequency = Quarterly;
        Real redemption = 100.0;
        Real faceAmount = 100.0;

        /* The 30/360 day counter Bloomberg uses for this bond cannot
           reproduce the US Bond/ISMA (constant) cashflows used in PFC1.
           Therefore use ActAct(Bond)
        */
        DayCounter bondDayCounter = ActualActual(ActualActual::Bond);

        // PFC1 shows no indication dates are being adjusted
        // for weekends/holidays for vanilla bonds
        BusinessDayConvention accrualConvention = Unadjusted;
        BusinessDayConvention paymentConvention = Unadjusted;

        Schedule sch(dated, maturity, Period(frequency), bondCalendar,
                     accrualConvention, accrualConvention,
                     DateGeneration::Backward, false);

        Size maxIterations = 1000;
        Real accuracy = 1e-8;
        Integer gridIntervals = 40;
        Real reversionParameter = .03;

        // output price/yield results for varying volatility parameter

        Real sigma = QL_EPSILON; // core dumps if zero on Cygwin

        auto hw0 = ext::make_shared<HullWhite>(termStructure,reversionParameter,sigma);

        auto engine0 = ext::make_shared<TreeCallableFixedRateBondEngine>(hw0,gridIntervals);

        CallableFixedRateBond callableBond(settlementDays, faceAmount, sch,
                                           vector<Rate>(1, coupon),
                                           bondDayCounter, paymentConvention,
                                           redemption, issue, callSchedule);
        callableBond.setPricingEngine(engine0);

        cout << setprecision(2)
             << showpoint
             << fixed
             << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib price/yld (%)  ";
        cout << callableBond.cleanPrice() << " / "
             << 100. * callableBond.yield(bondDayCounter,
                                          Compounded,
                                          frequency,
                                          accuracy,
                                          maxIterations)
             << endl;

        cout << "Bloomberg price/yld (%) ";
        cout << "96.50 / 5.47"
             << endl
             << endl;

        sigma = .01;

        cout << "sigma/vol (%) = " << 100.*sigma << endl;

        auto hw1 = ext::make_shared<HullWhite>(termStructure,reversionParameter,sigma);

        auto engine1 = ext::make_shared<TreeCallableFixedRateBondEngine>(hw1,gridIntervals);

        callableBond.setPricingEngine(engine1);

        cout << "QuantLib price/yld (%)  ";
        cout << callableBond.cleanPrice() << " / "
             << 100.* callableBond.yield(bondDayCounter,
                                         Compounded,
                                         frequency,
                                         accuracy,
                                         maxIterations)
             << endl;

        cout << "Bloomberg price/yld (%) ";
        cout << "95.68 / 5.66"
             << endl
             << endl;

        ////////////////////

        sigma = .03;

        auto hw2 = ext::make_shared<HullWhite>(termStructure, reversionParameter, sigma);

        auto engine2 = ext::make_shared<TreeCallableFixedRateBondEngine>(hw2,gridIntervals);

        callableBond.setPricingEngine(engine2);

        cout << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib price/yld (%)  ";
        cout << callableBond.cleanPrice() << " / "
             << 100. * callableBond.yield(bondDayCounter,
                                          Compounded,
                                          frequency,
                                          accuracy,
                                          maxIterations)
             << endl;

        cout << "Bloomberg price/yld (%) ";
        cout << "92.34 / 6.49"
             << endl
             << endl;

        ////////////////////////////

        sigma = .06;

        auto hw3 = ext::make_shared<HullWhite>(termStructure, reversionParameter, sigma);

        auto engine3 = ext::make_shared<TreeCallableFixedRateBondEngine>(hw3,gridIntervals);

        callableBond.setPricingEngine(engine3);

        cout << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib price/yld (%)  ";
        cout << callableBond.cleanPrice() << " / "
             << 100. * callableBond.yield(bondDayCounter,
                                          Compounded,
                                          frequency,
                                          accuracy,
                                          maxIterations)
             << endl;

        cout << "Bloomberg price/yld (%) ";
        cout << "87.16 / 7.83"
             << endl
             << endl;

        /////////////////////////

        sigma = .12;

        auto hw4 = ext::make_shared<HullWhite>(termStructure, reversionParameter, sigma);

        auto engine4 = ext::make_shared<TreeCallableFixedRateBondEngine>(hw4,gridIntervals);

        callableBond.setPricingEngine(engine4);

        cout << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib price/yld (%)  ";
        cout << callableBond.cleanPrice() << " / "
             << 100.* callableBond.yield(bondDayCounter,
                                         Compounded,
                                         frequency,
                                         accuracy,
                                         maxIterations)
             << endl;

        cout << "Bloomberg price/yld (%) ";
        cout << "77.31 / 10.65"
             << endl
             << endl;

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

