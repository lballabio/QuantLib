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
        Currency currency = EUR;
        int settlementDays = 2;
        Date todaysDate(6, November, 2001);


        /*********************
         ***  MARKET DATA  ***
         *********************/
        
        // market elements are containers for quotes.
        // SimpleMarketElement stores a value which can be manually changed;
        // other MarketElement subclasses could read the value from a 
        // database or some kind of data feed.

        // deposits
        Handle<MarketElement> d1wRate(new SimpleMarketElement(0.0382));
        Handle<MarketElement> d1mRate(new SimpleMarketElement(0.0372));
        Handle<MarketElement> d3mRate(new SimpleMarketElement(0.0363));
        Handle<MarketElement> d6mRate(new SimpleMarketElement(0.0353));
        Handle<MarketElement> d9mRate(new SimpleMarketElement(0.0348));
        Handle<MarketElement> d1yRate(new SimpleMarketElement(0.0345));
        // FRAs
        Handle<MarketElement> fra3x6Rate(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fra6x9Rate(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fra6x12Rate(new SimpleMarketElement(0.037125));
        // futures
        Handle<MarketElement> fut1Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut2Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut3Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut4Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut5Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut6Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut7Price(new SimpleMarketElement(0.037125));
        Handle<MarketElement> fut8Price(new SimpleMarketElement(0.037125));
        // swaps
        Handle<MarketElement> s2yRate(new SimpleMarketElement(0.037125));
        Handle<MarketElement> s3yRate(new SimpleMarketElement(0.0398));
        Handle<MarketElement> s5yRate(new SimpleMarketElement(0.0443));
        Handle<MarketElement> s10yRate(new SimpleMarketElement(0.05165));
        Handle<MarketElement> s15yRate(new SimpleMarketElement(0.055175));

        


        /*********************
         ***  RATE HELPERS ***
         *********************/
        
        // RateHelpers are built from the above quotes together with other
        // instrument dependant infos. Quotes are passed in relinkable handles
        // which could be relinked to some other data source later.

        // setup deposits
        Handle<DayCounter> depositDayCounter(new Actual360);

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

        // setup FRAs
        Handle<RateHelper> fra3x6(new FraRateHelper(
            RelinkableHandle<MarketElement>(fra3x6Rate),
            settlementDays, 3, 6, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fra6x9(new FraRateHelper(
            RelinkableHandle<MarketElement>(fra6x9Rate),
            settlementDays, 6, 9, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fra6x12(new FraRateHelper(
            RelinkableHandle<MarketElement>(fra6x12Rate),
            settlementDays, 6, 12, calendar, ModifiedFollowing,
            depositDayCounter));
        
        // setup futures
        // futures on 3 months deposit
        int futMonths = 3;
        Handle<RateHelper> fut1(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(19, December, 2001),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut2(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(20, March, 2002),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut3(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(19, June, 2002),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut4(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(18, September, 2002),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut5(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(18, December, 2002),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut6(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(19, March, 2003),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut7(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(18, June, 2003),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        Handle<RateHelper> fut8(new FuturesRateHelper(
            RelinkableHandle<MarketElement>(fut1Price),
            Date(17, September, 2003),
            settlementDays, futMonths, calendar, ModifiedFollowing,
            depositDayCounter));

        // setup swaps
        int swFixedLegFrequency = 1;
        bool swFixedLegIsAdjusted = false;
        Handle<DayCounter> swFixedLegDayCounter(new
            Thirty360(Thirty360::European));
        int swFloatingLegFrequency = 2;

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


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        Handle<DayCounter> termStructureDayCounter(
            new ActualActual(ActualActual::ISDA));


        // A depo-futures-swap curve
        std::vector<Handle<RateHelper> > depoFutSwapInstruments;
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
        Handle<TermStructure> depoFutSwapTermStructure(new
            PiecewiseFlatForward(currency, termStructureDayCounter,
            todaysDate, calendar, settlementDays, depoFutSwapInstruments));

/*

// adding the following 2 curves, even if unused, make it crash
// the 2 following curves do work if used instead of the above

        // A depo-swap curve
        std::vector<Handle<RateHelper> > depoSwapInstruments;
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
        Handle<TermStructure> depoSwapTermStructure(new
            PiecewiseFlatForward(currency, termStructureDayCounter,
            todaysDate, calendar, settlementDays, depoSwapInstruments));


        // A depo-FRA-swap curve
        std::vector<Handle<RateHelper> > depoFRASwapInstruments;
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
        Handle<TermStructure> depoFRASwapTermStructure(new
            PiecewiseFlatForward(currency, termStructureDayCounter,
            todaysDate, calendar, settlementDays, depoFRASwapInstruments));

*/

        
        /**********************
         * SWAPS TO BE PRICED *
         **********************/

        // Term structures that will be used for pricing:
        // the one used for discounting cash flows
        RelinkableHandle<TermStructure> discountingTermStructure;
        // the one used for forward rate forecasting
        RelinkableHandle<TermStructure> forecastingTermStructure;

        
        // spot start
        Date spotDate = calendar->advance(todaysDate, settlementDays, Days,
            Following);
        // constant nominal 1,000,000 Euro
        std::vector<double> nominals;
        nominals.push_back(1000000);
        // fixed leg
        int fixedLegFrequency = 1; // annual
        bool fixedLegIsAdjusted = false;
        RollingConvention roll = ModifiedFollowing;
        Handle<DayCounter> fixedLegDayCounter(new
            Thirty360(Thirty360::European));
        Rate fixedRate = 0.04;
        // constant coupon
        std::vector<double> couponRates;
        couponRates.push_back(fixedRate);
        
        // floating leg
        int floatingLegFrequency = 2;
        Handle<Xibor> euriborIndex(new Euribor(6, Months,
            forecastingTermStructure)); // using the forecasting curve
        // constant null spread
        std::vector<double> spreads;
        spreads.push_back(0.0);

        int lenghtInYears = 5;
        bool payFixedRate = true;
        SimpleSwap spot5YearSwap(payFixedRate, spotDate, lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads,
            discountingTermStructure); // using the discounting curve
        SimpleSwap oneYearForward5YearSwap(payFixedRate,
            calendar->advance(spotDate, 1, Years, ModifiedFollowing),
            lenghtInYears, Years,
            calendar, roll, nominals, fixedLegFrequency, couponRates,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, spreads,
            discountingTermStructure); // using the discounting curve



         /***************
         * SWAP PRICING *
         ****************/

        // let's price in term of NPV, fixed rate, and spread
        double NPV;
        Rate fairFixedRate;
        Spread fairFloatingSpread;

        // Of course, you're not forced to really use different curves
        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);
        std::cout << "*** using Depo-Fut-Swap term structure:" << std::endl;
        
        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               " 
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = - NPV / spot5YearSwap.floatingLegBPS();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " spread:               " 
            << RateFormatter::toString(fairFloatingSpread,8)
            << std::endl;
        fairFixedRate = fixedRate - NPV / spot5YearSwap.fixedLegBPS();
        std::cout << "5Y fixed rate:                  " 
            << RateFormatter::toString(fairFixedRate,8)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(abs(fairFixedRate-s5yRate->value())<1e-8,
            "5 years swap mispriced!");

        // now let's price the 1Y forward 5Y swap
        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            " 
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = -NPV / spot5YearSwap.floatingLegBPS();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " spread:            " 
            << RateFormatter::toString(fairFloatingSpread,8)
            << std::endl;
        fairFixedRate = fixedRate - NPV/oneYearForward5YearSwap.fixedLegBPS();
        std::cout << "1Yx5Y fixed rate:               " 
            << RateFormatter::toString(fairFixedRate,8)
            << std::endl;


        
        // let's say that the 5-years swap rate goes up to 4.60%.
        // A smarter market element--say, connected to a data source-- would 
        // notice the change itself. Since we're using SimpleMarketElements,
        // we'll have to change the value manually--which forces us to use
        // some ugly syntax to downcast and use the SimpleMarketElement 
        // interface. In any case, the point here is that a change in the
        // value contained in the MarketElement triggers a new bootstrapping
        // of the curve and a repricing of the swap.

        SimpleMarketElement* fiveYearsRate = 
        #if QL_ALLOW_TEMPLATE_METHOD_CALLS
            // Borland C++ and gcc
            s5yRate.downcast<SimpleMarketElement>();
        #else
            dynamic_cast<SimpleMarketElement*>(s5yRate.pointer());
        #endif
        fiveYearsRate->setValue(0.0460);
        std::cout <<  "*** 5Y swap goes up to 4.60%" << std::endl;
        

        // now get the updated results
        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               " 
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = - NPV / spot5YearSwap.floatingLegBPS();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " spread:               " 
            << RateFormatter::toString(fairFloatingSpread,8)
            << std::endl;
        fairFixedRate = fixedRate - NPV / spot5YearSwap.fixedLegBPS();
        std::cout << "5Y fixed rate:                  " 
            << RateFormatter::toString(fairFixedRate,8)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(abs(fairFixedRate-s5yRate->value())<1e-8,
            "5 years swap mispriced!");

        // now let's price the 1Y forward 5Y swap
        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            " 
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = -NPV / spot5YearSwap.floatingLegBPS();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " spread:            " 
            << RateFormatter::toString(fairFloatingSpread,8)
            << std::endl;
        fairFixedRate = fixedRate - NPV/oneYearForward5YearSwap.fixedLegBPS();
        std::cout << "1Yx5Y fixed rate:               " 
            << RateFormatter::toString(fairFixedRate,8)
            << std::endl;



        return 0;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

