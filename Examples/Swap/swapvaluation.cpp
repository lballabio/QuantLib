/*!
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

//! $Id$

/*  This example shows how to value a swap.
*/

// the only header you need to use QuantLib
#include "ql/quantlib.hpp"


// introducing the players ....

// Rate and Time are just double, but having their own types allows for
// a stronger check at compile time
using QuantLib::Rate;
using QuantLib::Time;

// Handle is the QuantLib way to have reference-counted objects
using QuantLib::Handle;
using QuantLib::RelinkableHandle;

using QuantLib::TermStructure;
using QuantLib::TermStructures::PiecewiseFlatForward;
using QuantLib::Currency;
using QuantLib::EUR;
using QuantLib::Date;
using QuantLib::Days;
using QuantLib::Months;
using QuantLib::Years;
using QuantLib::TimeUnit;
using QuantLib::Calendar;
using QuantLib::ModifiedFollowing;
using QuantLib::DayCounter;
using QuantLib::DayCounters::ActualActual;
using QuantLib::DayCounters::Actual360;
using QuantLib::DayCounters::Thirty360;
using QuantLib::October;
using QuantLib::Calendars::TARGET;
using QuantLib::TermStructures::RateHelper;
using QuantLib::TermStructures::DepositRateHelper;
using QuantLib::TermStructures::SwapRateHelper;
using QuantLib::RollingConvention;
using QuantLib::Instruments::SimpleSwap;
using QuantLib::Index;
using QuantLib::Indexes::Euribor;


// to format the output of doubles
using QuantLib::DoubleFormatter;



int main(int argc, char* argv[])
{
    try {
        Handle<Calendar> calendar(new TARGET);
        int settlementDays = 2;

        Handle<DayCounter> depositDayCounter(new Actual360);
        Handle<RateHelper> d1w(new DepositRateHelper(0.0382, settlementDays,
            1, Days, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d1m(new DepositRateHelper(0.0372, settlementDays,
            1, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d3m(new DepositRateHelper(0.0363, settlementDays,
            3, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d6m(new DepositRateHelper(0.0353, settlementDays,
            6, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d9m(new DepositRateHelper(0.0348, settlementDays,
            9, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d1y(new DepositRateHelper(0.0345, settlementDays,
            1, Years, calendar, ModifiedFollowing, depositDayCounter));


        int fixedLegFrequency = 1;
        bool fixedLegIsAdjusted = false;
        Handle<DayCounter> fixedLegDayCounter(new Thirty360(Thirty360::European));
        int floatingLegFrequency = 2;
        Handle<RateHelper> s2y(new SwapRateHelper(0.037125, settlementDays,
            2, calendar, ModifiedFollowing, fixedLegFrequency,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency));
        Handle<RateHelper> s3y(new SwapRateHelper(0.0398, settlementDays,
            3, calendar, ModifiedFollowing, fixedLegFrequency,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency));
        Handle<RateHelper> s5y(new SwapRateHelper(0.0443, settlementDays,
            5, calendar, ModifiedFollowing, fixedLegFrequency,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency));
        Handle<RateHelper> s10y(new SwapRateHelper(0.05165, settlementDays,
            10, calendar, ModifiedFollowing, fixedLegFrequency,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency));
        Handle<RateHelper> s15y(new SwapRateHelper(0.055175, settlementDays,
            15, calendar, ModifiedFollowing, fixedLegFrequency,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency));

        std::vector<Handle<RateHelper> > instruments;
        instruments.push_back(d1w);
        instruments.push_back(d1m);
        instruments.push_back(d3m);
        instruments.push_back(d6m);
        instruments.push_back(d9m);
        instruments.push_back(d1y);
        instruments.push_back(s2y);
        instruments.push_back(s3y);
        instruments.push_back(s5y);
        instruments.push_back(s10y);
        instruments.push_back(s15y);

        Currency currency(EUR);
        Handle<DayCounter> termStructureDayCounter(
            new ActualActual(ActualActual::ISDA));
        Date todaysDate(1, October, 2001);

        Handle<TermStructure> myTermStructure(new
            PiecewiseFlatForward(currency,
            termStructureDayCounter, todaysDate, calendar, settlementDays,
            instruments));

        RelinkableHandle<TermStructure > discountCurve;


        bool payFixedRate = true;
        Date startDate(3, October, 2001);
//        Date startDate = PiecewiseFlatForward.settlementDate();
        int lenghtInYears = 5;
        RollingConvention roll = ModifiedFollowing;
        std::vector<double> nominals;
        nominals.push_back(100000);
        fixedLegFrequency = 1;
        std::vector<double> couponRates;
        couponRates.push_back(0.0443);
        fixedLegIsAdjusted = false;
        floatingLegFrequency = 2;
/*
        Handle<Index> euriborIndex(new Euribor(6, Months, 
                const RelinkableHandle<TermStructure>& h)
*/
      Handle<Index> euriborIndex(new Euribor(6, Months, 
                discountCurve));
        std::vector<double> spreads;
        spreads.push_back(0.0);
        
        SimpleSwap mySwap(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads, discountCurve);
            
            
        discountCurve.linkTo(myTermStructure);
        std::cout << DoubleFormatter::toString(mySwap.NPV(),2) << std::endl;
        
        return 0;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

