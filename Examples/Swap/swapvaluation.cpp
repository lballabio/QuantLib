
/*!
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
        Calendar calendar = TARGET();
        Currency currency = EUR;
        int settlementDays = 2;
        int fixingDays = 2;

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Date todaysDate(6, November, 2001);

        // deposits
        double d1wQuote=0.0382;
        double d1mQuote=0.0372;
        double d3mQuote=0.0363;
        double d6mQuote=0.0353;
        double d9mQuote=0.0348;
        double d1yQuote=0.0345;
        // FRAs
        double fra3x6Quote=0.037125;
        double fra6x9Quote=0.037125;
        double fra6x12Quote=0.037125;
        // futures
        double fut1Quote=96.2875;
        double fut2Quote=96.7875;
        double fut3Quote=96.9875;
        double fut4Quote=96.6875;
        double fut5Quote=96.4875;
        double fut6Quote=96.3875;
        double fut7Quote=96.2875;
        double fut8Quote=96.0875;
        // swaps
        double s2yQuote=0.037125;
        double s3yQuote=0.0398;
        double s5yQuote=0.0443;
        double s10yQuote=0.05165;
        double s15yQuote=0.055175;




        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with other
        // instrument dependant infos.

        // setup deposits
        DayCounter depositDayCounter = Actual360();

        Handle<RateHelper> d1w(new DepositRateHelper(
            d1wQuote, settlementDays,
            1, Weeks, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d1m(new DepositRateHelper(
            d1mQuote, settlementDays,
            1, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d3m(new DepositRateHelper(
            d3mQuote, settlementDays,
            3, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d6m(new DepositRateHelper(
            d6mQuote, settlementDays,
            6, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d9m(new DepositRateHelper(
            d9mQuote, settlementDays,
            9, Months, calendar, ModifiedFollowing, depositDayCounter));
        Handle<RateHelper> d1y(new DepositRateHelper(
            d1yQuote, settlementDays,
            1, Years, calendar, ModifiedFollowing, depositDayCounter));

        // setup swaps
        int swFixedLegFrequency = 1;
        bool swFixedLegIsAdjusted = false;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
        int swFloatingLegFrequency = 2;

        Handle<RateHelper> s2y(new SwapRateHelper(
            s2yQuote, settlementDays,
            2, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s3y(new SwapRateHelper(
            s3yQuote, settlementDays,
            3, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s5y(new SwapRateHelper(
            s5yQuote, settlementDays,
            5, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s10y(new SwapRateHelper(
            s10yQuote, settlementDays,
            10, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        Handle<RateHelper> s15y(new SwapRateHelper(
            s15yQuote, settlementDays,
            15, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        DayCounter termStructureDayCounter =
            ActualActual(ActualActual::ISDA);

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


         /*********************
         * SWAPS TO BE PRICED *
         **********************/

        // Term structures that will be used for pricing:
        // the one used for discounting cash flows
        RelinkableHandle<TermStructure> discountingTermStructure;
        // the one used for forward rate forecasting
        RelinkableHandle<TermStructure> forecastingTermStructure;


        // spot start
        Date spotDate = calendar.advance(todaysDate, settlementDays, Days,
            Following);
        // constant nominal 1,000,000 Euro
        double nominal = 1000000.0;
        // fixed leg
        int fixedLegFrequency = 1; // annual
        bool fixedLegIsAdjusted = false;
        RollingConvention roll = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        Rate fixedRate = 0.04;

        // floating leg
        int floatingLegFrequency = 2;
        Handle<Xibor> euriborIndex(new Euribor(6, Months,
            forecastingTermStructure)); // using the forecasting curve
        Spread spread = 0.0;

        int lenghtInYears = 5;
        bool payFixedRate = true;
        SimpleSwap spot5YearSwap(payFixedRate, spotDate, lenghtInYears,
            Years, calendar, roll, nominal, fixedLegFrequency, fixedRate,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, fixingDays, spread,
            discountingTermStructure); // using the discounting curve
        SimpleSwap oneYearForward5YearSwap(payFixedRate,
            calendar.advance(spotDate, 1, Years, ModifiedFollowing),
            lenghtInYears, Years,
            calendar, roll, nominal, fixedLegFrequency, fixedRate,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, fixingDays, spread,
            discountingTermStructure); // using the discounting curve



         /***************
         * SWAP PRICING *
         ****************/

        std::cout <<  "*** 5Y swap at 4.30%" << std::endl;

        
        // let's price in term of NPV, fixed rate, and spread
        double NPV;
        Rate fairFixedRate;
        Spread fairFloatingSpread;

        // Of course, you're not forced to really use different curves
        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(depoSwapTermStructure);
        std::cout << "*** using Depo-Swap term structure:" << std::endl;

        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = spot5YearSwap.fairSpread();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:          "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = spot5YearSwap.fairRate();
        std::cout << "5Y fair fixed rate:             "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(QL_FABS(fairFixedRate-s5yQuote)<1e-8,
            "5 years swap mispriced!");

        // now let's price the 1Y forward 5Y swap
        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = oneYearForward5YearSwap.fairSpread();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:       "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = oneYearForward5YearSwap.fairRate();
        std::cout << "1Yx5Y fair fixed rate:          "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;






        // now, let's get serious

        /*********************
         ***  MARKET DATA  ***
         *********************/

        // market elements are containers for quotes.
        // SimpleMarketElement stores a value which can be manually changed;
        // other MarketElement subclasses could read the value from a
        // database or some kind of data feed.

        // deposits
        Handle<MarketElement> d1wRate(new SimpleMarketElement(d1wQuote));
        Handle<MarketElement> d1mRate(new SimpleMarketElement(d1mQuote));
        Handle<MarketElement> d3mRate(new SimpleMarketElement(d3mQuote));
        Handle<MarketElement> d6mRate(new SimpleMarketElement(d6mQuote));
        Handle<MarketElement> d9mRate(new SimpleMarketElement(d9mQuote));
        Handle<MarketElement> d1yRate(new SimpleMarketElement(d1yQuote));
        // FRAs
        Handle<MarketElement> fra3x6Rate(new SimpleMarketElement(fra3x6Quote));
        Handle<MarketElement> fra6x9Rate(new SimpleMarketElement(fra6x9Quote));
        Handle<MarketElement> fra6x12Rate(new SimpleMarketElement(fra6x12Quote));
        // futures
        Handle<MarketElement> fut1Price(new SimpleMarketElement(fut1Quote));
        Handle<MarketElement> fut2Price(new SimpleMarketElement(fut2Quote));
        Handle<MarketElement> fut3Price(new SimpleMarketElement(fut3Quote));
        Handle<MarketElement> fut4Price(new SimpleMarketElement(fut4Quote));
        Handle<MarketElement> fut5Price(new SimpleMarketElement(fut5Quote));
        Handle<MarketElement> fut6Price(new SimpleMarketElement(fut6Quote));
        Handle<MarketElement> fut7Price(new SimpleMarketElement(fut7Quote));
        Handle<MarketElement> fut8Price(new SimpleMarketElement(fut8Quote));
        // swaps
        Handle<MarketElement> s2yRate(new SimpleMarketElement(s2yQuote));
        Handle<MarketElement> s3yRate(new SimpleMarketElement(s3yQuote));
        Handle<MarketElement> s5yRate(new SimpleMarketElement(s5yQuote));
        Handle<MarketElement> s10yRate(new SimpleMarketElement(s10yQuote));
        Handle<MarketElement> s15yRate(new SimpleMarketElement(s15yQuote));






        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with other
        // instrument dependant infos.
        // This time quotes are passed in relinkable
        // handles which could be relinked to some other data source later.

        // setup deposits
        d1w =Handle<RateHelper>(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d1wRate), settlementDays,
            1, Weeks, calendar, ModifiedFollowing, depositDayCounter));
        d1m=Handle<RateHelper>(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d1mRate), settlementDays,
            1, Months, calendar, ModifiedFollowing, depositDayCounter));
        d3m=Handle<RateHelper>(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d3mRate), settlementDays,
            3, Months, calendar, ModifiedFollowing, depositDayCounter));
        d6m=Handle<RateHelper>(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d6mRate), settlementDays,
            6, Months, calendar, ModifiedFollowing, depositDayCounter));
        d9m=Handle<RateHelper>(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d9mRate), settlementDays,
            9, Months, calendar, ModifiedFollowing, depositDayCounter));
        d1y=Handle<RateHelper>(new DepositRateHelper(
            RelinkableHandle<MarketElement>(d1yRate), settlementDays,
            1, Years, calendar, ModifiedFollowing, depositDayCounter));

        // setup swaps
        s2y=Handle<RateHelper>(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s2yRate), settlementDays,
            2, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        s3y=Handle<RateHelper>(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s3yRate), settlementDays,
            3, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        s5y=Handle<RateHelper>(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s5yRate), settlementDays,
            5, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        s10y=Handle<RateHelper>(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s10yRate), settlementDays,
            10, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        s15y=Handle<RateHelper>(new SwapRateHelper(
            RelinkableHandle<MarketElement>(s15yRate), settlementDays,
            15, calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));


        // let's add FRA and futures

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






        /*********************
         **  CURVE BUILDING **
         *********************/

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






         /***************
         * SWAP PRICING *
         ****************/


        // switch the curve used by the swaps to be priced
        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);
        std::cout << "*** using Depo-Fut-Swap term structure:" << std::endl;

        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = spot5YearSwap.fairSpread();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:          "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = spot5YearSwap.fairRate();
        std::cout << "5Y fair fixed rate:             "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(QL_FABS(fairFixedRate-s5yQuote)<1e-8,
            "5 years swap mispriced!");

        // now let's price the 1Y forward 5Y swap
        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = oneYearForward5YearSwap.fairSpread();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:       "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = oneYearForward5YearSwap.fairRate();
        std::cout << "1Yx5Y fair fixed rate:          "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;


        // switch the curve used by the swaps to be priced
        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);
        std::cout << "*** using Depo-FRA-Swap term structure:" << std::endl;

        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = spot5YearSwap.fairSpread();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:          "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = spot5YearSwap.fairRate();
        std::cout << "5Y fair fixed rate:             "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(QL_FABS(fairFixedRate-s5yRate->value())<1e-8,
            "5 years swap mispriced!");

        // now let's price the 1Y forward 5Y swap
        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = oneYearForward5YearSwap.fairSpread();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:       "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = oneYearForward5YearSwap.fairRate();
        std::cout << "1Yx5Y fair fixed rate:          "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;


        // now let's say that the 5-years swap rate goes up to 4.60%.
        // A smarter market element--say, connected to a data source-- would
        // notice the change itself. Since we're using SimpleMarketElements,
        // we'll have to change the value manually--which forces us to
        // downcast the handle and use the SimpleMarketElement
        // interface. In any case, the point here is that a change in the
        // value contained in the MarketElement triggers a new bootstrapping
        // of the curve and a repricing of the swap.

        Handle<SimpleMarketElement> fiveYearsRate = s5yRate;
        fiveYearsRate->setValue(0.0460);
        std::cout << std::endl <<  "*** 5Y swap goes up to 4.60%" << std::endl;


        // now get the updated results
        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);
        std::cout << "*** using Depo-Fut-Swap term structure:" << std::endl;

        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = spot5YearSwap.fairSpread();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:          "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = spot5YearSwap.fairRate();
        std::cout << "5Y fair fixed rate:             "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(QL_FABS(fairFixedRate-s5yRate->value())<1e-8,
            "5 years swap mispriced!");

        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = oneYearForward5YearSwap.fairSpread();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:       "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = oneYearForward5YearSwap.fairRate();
        std::cout << "1Yx5Y fair fixed rate:          "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);
        std::cout << "*** using Depo-FRA-Swap term structure:" << std::endl;

        NPV = spot5YearSwap.NPV();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:               "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = spot5YearSwap.fairSpread();
        std::cout << "5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:          "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = spot5YearSwap.fairRate();
        std::cout << "5Y fair fixed rate:             "
            << RateFormatter::toString(fairFixedRate,4)
            << std::endl;
        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(QL_FABS(fairFixedRate-s5yRate->value())<1e-8,
            "5 years swap mispriced!");

        NPV = oneYearForward5YearSwap.NPV();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " NPV:            "
            << DoubleFormatter::toString(NPV,2)
            << std::endl;
        fairFloatingSpread = oneYearForward5YearSwap.fairSpread();
        std::cout << "1Yx5Y "
            << RateFormatter::toString(fixedRate,2)
            << " fair spread:       "
            << RateFormatter::toString(fairFloatingSpread,4)
            << std::endl;
        fairFixedRate = oneYearForward5YearSwap.fairRate();
        std::cout << "1Yx5Y fair fixed rate:          "
            << RateFormatter::toString(fairFixedRate,4)
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

