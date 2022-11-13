/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2022 Oleg Kulkov

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

/*  This example shows how the amortization and draw downs of a bond 
    work
 */

#include <ql/qldefines.hpp>
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/schedule.hpp>

#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

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

        /***********************
         ***  SCHEDULE PREP  ***
         ***********************/

        Calendar calendar = TARGET();

        Date settlementDate(11, September, 2015);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);

        Integer fixingDays = 2;
        Natural settlementDays = 2;

        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
        // nothing to do with Date::todaysDate
        Settings::instance().evaluationDate() = todaysDate;

        std::cout << "Today: " << todaysDate.weekday()
        << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
        << ", " << settlementDate << std::endl;

        // setup bond
        Real redemption = 100.0;
        Date issueDate = Date (19, May, 2012);
        Date maturityDate = Date (25, May, 2042);
        Real couponRate = 0.042;
        Real marketQuote = 100.0;
        
        std::vector<Date> dateSchedule;

        //build semiannual schedule
        Schedule dateSchedule(issueDate, maturityDate, Period(Semiannual), UnitedStates(UnitedStates::GovernmentBond),
                    Unadjusted, Unadjusted, DateGeneration::Backward, false);

        //define nominals vector with the lenght equal to the number of dates
        std::vector<double> nominals(dateSchedule.size());
        std::vector<double> rates(dateSchedule.size(),couponRate);
        //fill out nominals
        //on third schedule date the notional will increase by 0.5
        //on last before maturity the notional will decrease by 0.5
        for (Size i = 0; i < dateSchedule.size(); ++i) {
            ((i < 2) || (i >= dateSchedule.size() - 2)) ? 
                nominals.push_back(0.5) : nominals.push_back(1.0);
        }

        boost::shared_ptr<SimpleQuote> cp(new SimpleQuote(marketQuote));

        RelinkableHandle<Quote> quoteHandle;
        quoteHandle.linkTo(cp);

        // Definition of the rate helpers
        std::vector<boost::shared_ptr<BondHelper>> bondsHelper;

        Schedule schedule(issueDate, maturityDate, Period(Semiannual), UnitedStates(UnitedStates::GovernmentBond),
                    Unadjusted, Unadjusted, DateGeneration::Backward, false);

        Leg leg = FixedRateLeg(schedule)
                  .withNotionals(nominals)
                  .withCouponRates(rates,Actual360())
                  .withPaymentAdjustment(Unadjusted)
                  .withPaymentCalendar(calendar);

        Bond fixedRateBond(settlementDays,calendar,issueDate,leg);

         /***************
          * BOND OUTPUT *
          ****************/

        Leg legBond = fixedRateBond.cashflows();
        for (Size i = 0; i < legBond.size(); ++i){
            auto bondFlow = legBond[i];
            auto flowCoupon = boost::dynamic_pointer_cast<Coupon>(bondFlow);
            std::cout << std::fixed << std::setprecision(2) << bondFlow->amount() <<
            bondFlow->date();
        }

         std::cout << std::endl;

         // write column headings
         Size widths[] = { 18, 10, 10, 10 };

         std::cout << std::setw(widths[0]) <<  "                 "
         << std::setw(widths[1]) << "ZC"
         << std::setw(widths[2]) << "Fixed"
         << std::setw(widths[3]) << "Floating"
         << std::endl;

         Size width = widths[0] + widths[1] + widths[2] + widths[3];
         std::string rule(width, '-');

         std::cout << rule << std::endl;

         std::cout << std::fixed;
         std::cout << std::setprecision(2);

         double seconds = timer.elapsed();
         Integer hours = int(seconds/3600);
         seconds -= hours * 3600;
         Integer minutes = int(seconds/60);
         seconds -= minutes * 60;
         std::cout << " \nRun completed in ";
         if (hours > 0)
             std::cout << hours << " h ";
         if (hours > 0 || minutes > 0)
             std::cout << minutes << " m ";
         std::cout << std::fixed << std::setprecision(0)
         << seconds << " s\n" << std::endl;

         return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
