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

/*  This example shows how to set up a Term Structure and then price a simple
    swap.
*/

// the only header you need to use QuantLib
#include <ql/quantlib.hpp>


using namespace QuantLib;
using Calendars::TARGET;
using DayCounters::ActualActual;
using DayCounters::Actual360;
using DayCounters::Thirty360;
using Indexes::Xibor;
using Indexes::Euribor;
using Instruments::SimpleSwap;
using TermStructures::PiecewiseFlatForward;
using TermStructures::FlatForward;
using TermStructures::RateHelper;
using TermStructures::DepositRateHelper;
using TermStructures::FraRateHelper;
using TermStructures::FuturesRateHelper;
using TermStructures::SwapRateHelper;


int main(int argc, char* argv[])
{
    try {
        Handle<Calendar> calendar(new TARGET);
        int settlementDays = 2;
        Date todaysDate(1, October, 2001);

        // setup deposits
        Handle<DayCounter> depositDayCounter(new Actual360);

        // market elements are containers for quotes.
        // SimpleMarketElement stores a value which can be manually changed;
        // other MarketElement subclasses could read the value from a 
        // database or some kind of data feed.
        Handle<MarketElement> d1wRate(new SimpleMarketElement(0.0382));
        Handle<MarketElement> d1mRate(new SimpleMarketElement(0.0372));
        Handle<MarketElement> d3mRate(new SimpleMarketElement(0.0363));
        Handle<MarketElement> d6mRate(new SimpleMarketElement(0.0353));
        Handle<MarketElement> d9mRate(new SimpleMarketElement(0.0348));
        Handle<MarketElement> d1yRate(new SimpleMarketElement(0.0345));
        
        // RateHelpers are built from the above quotes together with other
        // deposit infos. Quotes are passed in relinkable handles which
        // could be relinked to some other data source later.
        Handle<RateHelper> d1w(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d1wRate), settlementDays,
            1, Days, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d1m(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d1mRate), settlementDays,
            1, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d3m(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d3mRate), settlementDays,
            3, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d6m(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d6mRate), settlementDays,
            6, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d9m(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d9mRate), settlementDays,
            9, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d1y(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d1yRate), settlementDays,
            1, Years, calendar, ModifiedFollowing, depositDayCounter));


        // setup swaps
        
        int swFixedLegFrequency = 1;
        bool swFixedLegIsAdjusted = false;
        Handle<DayCounter> swFixedLegDayCounter(new
            Thirty360(Thirty360::European));
        int swFloatingLegFrequency = 2;

        Handle<MarketElement> s2yRate(new SimpleMarketElement(0.037125));
        Handle<MarketElement> s3yRate(new SimpleMarketElement(0.0398));
        Handle<MarketElement> s5yRate(new SimpleMarketElement(0.0443));
        Handle<MarketElement> s10yRate(new SimpleMarketElement(0.05165));
        Handle<MarketElement> s15yRate(new SimpleMarketElement(0.055175));

        Handle<RateHelper> s2y(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s2yRate), settlementDays,
            2, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s3y(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s3yRate), settlementDays,
            3, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s5y(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s5yRate), settlementDays,
            5, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s10y(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s10yRate), settlementDays,
            10, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s15y(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s15yRate), settlementDays,
            15, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));

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

        Currency currency = EUR;
        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        Handle<DayCounter> termStructureDayCounter(
            new ActualActual(ActualActual::ISDA));

        // create the curve
        Handle<TermStructure> myTermStructure(new
            PiecewiseFlatForward(currency, termStructureDayCounter,
            todaysDate, calendar, settlementDays, instruments));

        // let's start with the swap pricing

        // this will be used for discounting and for
        // forward rate forecasting
        // Of course, you're not forced to use the same curve
        RelinkableHandle<TermStructure> rhTermStructure;

        // spot start
        Date startDate = myTermStructure->settlementDate();
        int lenghtInYears = 5;
        std::vector<double> nominals;
        nominals.push_back(1);

        // fixed leg
        int fixedLegFrequency = 1;
        bool fixedLegIsAdjusted = false;
        RollingConvention roll = ModifiedFollowing;
        Handle<DayCounter> fixedLegDayCounter(new
            Thirty360(Thirty360::European));
        Rate fixedRate = 0.04; // dummy value
        std::vector<double> couponRates;
        couponRates.push_back(fixedRate);

        // floating leg
        int floatingLegFrequency = 2;
        Handle<Xibor> euriborIndex(new Euribor(6, Months,
            rhTermStructure));
        std::vector<double> spreads;
        spreads.push_back(0.0);

        bool payFixedRate = true;
        SimpleSwap mySwap(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads, rhTermStructure);


        rhTermStructure.linkTo(myTermStructure);
        Rate fairFixedRate = fixedRate-mySwap.NPV()/mySwap.fixedLegBPS();
        Spread fairFloatingSpread = -mySwap.NPV()/mySwap.floatingLegBPS();

        SimpleSwap testSwap1(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, 
            std::vector<double>(1, fairFixedRate),  // pass fair fixed rate
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads, rhTermStructure);
        SimpleSwap testSwap2(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, 
            std::vector<double>(1, fairFloatingSpread),  // pass fair spread
            rhTermStructure);

        std::cout << "Swap priced on given term structure:" << std::endl;
        std::cout << "Fair fixed rate:              " 
            << RateFormatter::toString(fairFixedRate,8)
            << std::endl;
        std::cout << "    (checked: NPV is " 
            << testSwap1.NPV()
            << " per unit nominal)" << std::endl;
        std::cout << "Fair spread on floating leg: " 
            << RateFormatter::toString(fairFloatingSpread,8) 
            << std::endl;
        std::cout << "    (checked: NPV is " 
            << testSwap2.NPV()
            << " per unit nominal)" << std::endl << std::endl;


        // let's say that the 5-years swap rate goes up to 4.60%.
        // A smarter market element--say, connected to a data source-- would 
        // notice the change itself. Since we're using SimpleMarketElements,
        // we'll have to change the value manually--which forces us to use
        // some ugly syntax to downcast and use the SimpleMarketElement 
        // interface. In any case, the change in the value contained in the
        // MarketElement triggers a new bootstrapping of the curve and a 
        // repricing of the swap.

        SimpleMarketElement* fiveYearsRate = 
        #if QL_ALLOW_TEMPLATE_METHOD_CALLS
            // Borland C++ and gcc
            s5yRate.downcast<SimpleMarketElement>();
        #else
            dynamic_cast<SimpleMarketElement*>(s5yRate.pointer());
        #endif
        fiveYearsRate->setValue(0.0460);
        
        // now get the updated results
        fairFixedRate = fixedRate-mySwap.NPV()/mySwap.fixedLegBPS();
        fairFloatingSpread = -mySwap.NPV()/mySwap.floatingLegBPS();

        SimpleSwap testSwap3(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, 
            std::vector<double>(1, fairFixedRate),  // pass fair fixed rate
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads, rhTermStructure);
        SimpleSwap testSwap4(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, 
            std::vector<double>(1, fairFloatingSpread),  // pass fair spread
            rhTermStructure);

        std::cout << "Swap priced on perturbed term structure:" << std::endl;
        std::cout << "Fair fixed rate:              " 
            << RateFormatter::toString(fairFixedRate,8)
            << std::endl;
        std::cout << "    (checked: NPV is " 
            << testSwap3.NPV()
            << " per unit nominal)" << std::endl;
        std::cout << "Fair spread on floating leg: " 
            << RateFormatter::toString(fairFloatingSpread,8) 
            << std::endl;
        std::cout << "    (checked: NPV is " 
            << testSwap4.NPV()
            << " per unit nominal)" << std::endl << std::endl;
        

        // let's price the same swap on a different term structure
        // e.g. Flat Forward at 5.0%
        Handle<TermStructure> newTermStructure(new
            FlatForward(currency, termStructureDayCounter, todaysDate,
            calendar, settlementDays, 0.05));

        rhTermStructure.linkTo(newTermStructure);
        fairFixedRate = fixedRate-mySwap.NPV()/mySwap.fixedLegBPS();
        fairFloatingSpread = -mySwap.NPV()/mySwap.floatingLegBPS();

        SimpleSwap testSwap5(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, 
            std::vector<double>(1, fairFixedRate),  // pass fair fixed rate
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads, rhTermStructure);
        SimpleSwap testSwap6(payFixedRate, startDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, 
            std::vector<double>(1, fairFloatingSpread),  // pass fair spread
            rhTermStructure);

        std::cout << "Swap priced on flat term structure:" << std::endl;
        std::cout << "Fair fixed rate:              " << 
            RateFormatter::toString(fairFixedRate,4) 
            << std::endl;
        std::cout << "    (checked: NPV is " 
            << testSwap5.NPV()
            << " per unit nominal)" << std::endl;
        std::cout << "Fair spread on floating leg: " << 
            RateFormatter::toString(fairFloatingSpread,4) 
            << std::endl;
        std::cout << "    (checked: NPV is " 
            << testSwap6.NPV()
            << " per unit nominal)" << std::endl;

        return 0;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

