/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2006 Allen Kuo

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

/* a Repo calculation done using the FixedRateBondForward class
   cf. aaBondFwd() repo example at
   http://www.fincad.com/support/developerFunc/mathref/BFWD.htm

   This repo is set up to use the repo rate to do all discounting
   (including the underlying bond income). Forward delivery price is
   also obtained using this repo rate. All this is done by supplying
   the FixedRateBondForward constructor with a flat repo
   YieldTermStructure.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/bondforward.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace QuantLib;

int main(int, char* []) {

    try {

        std::cout << std::endl;

        Date repoSettlementDate(14,February,2000);;
        Date repoDeliveryDate(15,August,2000);
        Rate repoRate = 0.05;
        DayCounter repoDayCountConvention = Actual360();
        Integer repoSettlementDays = 0;
        Compounding repoCompounding = Simple;
        Frequency repoCompoundFreq = Annual;

        // assume a ten year bond- this is irrelevant
        Date bondIssueDate(15,September,1995);
        Date bondDatedDate(15,September,1995);
        Date bondMaturityDate(15,September,2005);
        Real bondCoupon = 0.08;
        Frequency bondCouponFrequency = Semiannual;
        // unknown what calendar fincad is using
        Calendar bondCalendar = NullCalendar();
        DayCounter bondDayCountConvention = Thirty360(Thirty360::BondBasis);
        // unknown what fincad is using. this may affect accrued calculation
        Integer bondSettlementDays = 0;
        BusinessDayConvention bondBusinessDayConvention = Unadjusted;
        Real bondCleanPrice = 89.97693786;
        Real bondRedemption = 100.0;
        Real faceAmount = 100.0;


        Settings::instance().evaluationDate() = repoSettlementDate;

        RelinkableHandle<YieldTermStructure> bondCurve;
        bondCurve.linkTo(ext::make_shared<FlatForward>(repoSettlementDate,
                                                       .01, // dummy rate
                                                       bondDayCountConvention,
                                                       Compounded,
                                                       bondCouponFrequency));

        /*
        auto bond = ext::make_shared<FixedRateBond>(faceAmount,
                                         bondIssueDate,
                                         bondDatedDate,
                                         bondMaturityDate,
                                         bondSettlementDays,
                                         std::vector<Rate>(1,bondCoupon),
                                         bondCouponFrequency,
                                         bondCalendar,
                                         bondDayCountConvention,
                                         bondBusinessDayConvention,
                                         bondBusinessDayConvention,
                                         bondRedemption,
                                         bondCurve);
        */

        Schedule bondSchedule(bondDatedDate, bondMaturityDate,
                              Period(bondCouponFrequency),
                              bondCalendar,bondBusinessDayConvention,
                              bondBusinessDayConvention,
                              DateGeneration::Backward,false);
        auto bond = ext::make_shared<FixedRateBond>(bondSettlementDays,
                                         faceAmount,
                                         bondSchedule,
                                         std::vector<Rate>(1,bondCoupon),
                                         bondDayCountConvention,
                                         bondBusinessDayConvention,
                                         bondRedemption,
                                         bondIssueDate);
        bond->setPricingEngine(ext::make_shared<DiscountingBondEngine>(bondCurve));

        bondCurve.linkTo(ext::make_shared<FlatForward>(repoSettlementDate,
                                   bond->yield(bondCleanPrice,
                                               bondDayCountConvention,
                                               Compounded,
                                               bondCouponFrequency),
                                   bondDayCountConvention,
                                   Compounded,
                                   bondCouponFrequency));

        Position::Type fwdType = Position::Long;
        double dummyStrike = 91.5745;

        RelinkableHandle<YieldTermStructure> repoCurve;
        repoCurve.linkTo(ext::make_shared<FlatForward>(repoSettlementDate,
                                                       repoRate,
                                                       repoDayCountConvention,
                                                       repoCompounding,
                                                       repoCompoundFreq));


        BondForward bondFwd(repoSettlementDate, repoDeliveryDate, fwdType, dummyStrike,
                            repoSettlementDays, repoDayCountConvention, bondCalendar,
                            bondBusinessDayConvention, bond, repoCurve, repoCurve);


        cout << "Underlying bond clean price: "
             << bond->cleanPrice()
             << endl;
        cout << "Underlying bond dirty price: "
             << bond->dirtyPrice()
             << endl;
        cout << "Underlying bond accrued at settlement: "
             << bond->accruedAmount(repoSettlementDate)
             << endl;
        cout << "Underlying bond accrued at delivery:   "
             << bond->accruedAmount(repoDeliveryDate)
             << endl;
        cout << "Underlying bond spot income: "
             << bondFwd.spotIncome(repoCurve)
             << endl;
        cout << "Underlying bond fwd income:  "
             << bondFwd.spotIncome(repoCurve)/
                repoCurve->discount(repoDeliveryDate)
             << endl;
        cout << "Repo strike: "
             << dummyStrike
             << endl;
        cout << "Repo NPV:    "
             << bondFwd.NPV()
             << endl;
        cout << "Repo clean forward price: "
             << bondFwd.cleanForwardPrice()
             << endl;
        cout << "Repo dirty forward price: "
             << bondFwd.forwardPrice()
             << endl;
        cout << "Repo implied yield: "
             << bondFwd.impliedYield(bond->dirtyPrice(),
                                     dummyStrike,
                                     repoSettlementDate,
                                     repoCompounding,
                                     repoDayCountConvention)
             << endl;
        cout << "Market repo rate:   "
             << repoCurve->zeroRate(repoDeliveryDate,
                                    repoDayCountConvention,
                                    repoCompounding,
                                    repoCompoundFreq)
             << endl
             << endl;

        cout << "Compare with example given at \n"
             << "http://www.fincad.com/support/developerFunc/mathref/BFWD.htm"
             <<  endl;
        cout << "Clean forward price = 88.2408"
             <<  endl
             <<  endl;
        cout << "In that example, it is unknown what bond calendar they are\n"
             << "using, as well as settlement Days. For that reason, I have\n"
             << "made the simplest possible assumptions here: NullCalendar\n"
             << "and 0 settlement days."
             << endl;


        return 0;

    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

