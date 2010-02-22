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

// the only header you need to use QuantLib
#include <ql/quantlib.hpp>

#ifdef BOOST_MSVC
/* Uncomment the following lines to unmask floating-point
   exceptions. Warning: unpredictable results can arise...

   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
   Is there anyone with a definitive word about this?
*/
// #include <float.h>
// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

#include <boost/timer.hpp>
#include <iostream>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif

int main(int, char* []) {

    try {

        boost::timer timer;
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
        bondCurve.linkTo(boost::shared_ptr<YieldTermStructure>(
                                       new FlatForward(repoSettlementDate,
                                                       .01, // dummy rate
                                                       bondDayCountConvention,
                                                       Compounded,
                                                       bondCouponFrequency)));

        /*
        boost::shared_ptr<FixedRateBond> bond(
                       new FixedRateBond(faceAmount,
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
                                         bondCurve));
        */

        Schedule bondSchedule(bondDatedDate, bondMaturityDate,
                              Period(bondCouponFrequency),
                              bondCalendar,bondBusinessDayConvention,
                              bondBusinessDayConvention,
                              DateGeneration::Backward,false);
        boost::shared_ptr<FixedRateBond> bond(
                       new FixedRateBond(bondSettlementDays,
                                         faceAmount,
                                         bondSchedule,
                                         std::vector<Rate>(1,bondCoupon),
                                         bondDayCountConvention,
                                         bondBusinessDayConvention,
                                         bondRedemption,
                                         bondIssueDate));
        bond->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                       new DiscountingBondEngine(bondCurve)));

        bondCurve.linkTo(boost::shared_ptr<YieldTermStructure> (
                   new FlatForward(repoSettlementDate,
                                   bond->yield(bondCleanPrice,
                                               bondDayCountConvention,
                                               Compounded,
                                               bondCouponFrequency),
                                   bondDayCountConvention,
                                   Compounded,
                                   bondCouponFrequency)));

        Position::Type fwdType = Position::Long;
        double dummyStrike = 91.5745;

        RelinkableHandle<YieldTermStructure> repoCurve;
        repoCurve.linkTo(boost::shared_ptr<YieldTermStructure> (
                                       new FlatForward(repoSettlementDate,
                                                       repoRate,
                                                       repoDayCountConvention,
                                                       repoCompounding,
                                                       repoCompoundFreq)));


        FixedRateBondForward bondFwd(repoSettlementDate,
                                     repoDeliveryDate,
                                     fwdType,
                                     dummyStrike,
                                     repoSettlementDays,
                                     repoDayCountConvention,
                                     bondCalendar,
                                     bondBusinessDayConvention,
                                     bond,
                                     repoCurve,
                                     repoCurve);


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


        Real seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        cout << " \nRun completed in ";
        if (hours > 0)
            cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            cout << minutes << " m ";
        cout << fixed << setprecision(0)
             << seconds << " s\n" << endl;

        return 0;

    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

