
/*!
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*  This example shows how to set up a Term Structure and then price a simple
    swap.
*/

// the only header you need to use QuantLib
#include <ql/quantlib.hpp>
#include <iomanip>

using namespace QuantLib;

int main(int, char* [])
{
    try {
        QL_IO_INIT

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();
        Date todaysDate(6, November, 2001);
        Date settlementDate(8, November, 2001);

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


        /********************
         ***    QUOTES    ***
         ********************/

        // SimpleQuote stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.

        // deposits
        boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
        boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
        boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
        boost::shared_ptr<Quote> d6mRate(new SimpleQuote(d6mQuote));
        boost::shared_ptr<Quote> d9mRate(new SimpleQuote(d9mQuote));
        boost::shared_ptr<Quote> d1yRate(new SimpleQuote(d1yQuote));
        // FRAs
        boost::shared_ptr<Quote> fra3x6Rate(new SimpleQuote(fra3x6Quote));
        boost::shared_ptr<Quote> fra6x9Rate(new SimpleQuote(fra6x9Quote));
        boost::shared_ptr<Quote> fra6x12Rate(new SimpleQuote(fra6x12Quote));
        // futures
        boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));
        boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));
        boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));
        boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));
        boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));
        boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));
        boost::shared_ptr<Quote> fut7Price(new SimpleQuote(fut7Quote));
        boost::shared_ptr<Quote> fut8Price(new SimpleQuote(fut8Quote));
        // swaps
        boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
        boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
        boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
        boost::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));
        boost::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));


        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // deposits
        DayCounter depositDayCounter = Actual360();
        int settlementDays = 2;

        boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(
            RelinkableHandle<Quote>(d1wRate), 
            1, Weeks, settlementDays, 
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(
            RelinkableHandle<Quote>(d1mRate), 
            1, Months, settlementDays, 
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(
            RelinkableHandle<Quote>(d3mRate), 
            3, Months, settlementDays, 
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d6m(new DepositRateHelper(
            RelinkableHandle<Quote>(d6mRate), 
            6, Months, settlementDays, 
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d9m(new DepositRateHelper(
            RelinkableHandle<Quote>(d9mRate), 
            9, Months, settlementDays, 
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d1y(new DepositRateHelper(
            RelinkableHandle<Quote>(d1yRate), 
            1, Years, settlementDays, 
            calendar, ModifiedFollowing, depositDayCounter));


        // setup FRAs
        boost::shared_ptr<RateHelper> fra3x6(new FraRateHelper(
            RelinkableHandle<Quote>(fra3x6Rate),
            3, 6, settlementDays, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x9(new FraRateHelper(
            RelinkableHandle<Quote>(fra6x9Rate),
            6, 9, settlementDays, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x12(new FraRateHelper(
            RelinkableHandle<Quote>(fra6x12Rate),
            6, 12, settlementDays, calendar, ModifiedFollowing,
            depositDayCounter));


        // setup futures
        int futMonths = 3;
        boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(19, December, 2001),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(20, March, 2002),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(19, June, 2002),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(18, September, 2002),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(18, December, 2002),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(19, March, 2003),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(18, June, 2003),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(
            RelinkableHandle<Quote>(fut1Price),
            Date(17, September, 2003),
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter));


        // setup swaps
        int swFixedLegFrequency = 1;
        bool swFixedLegIsAdjusted = false;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
        int swFloatingLegFrequency = 2;

        boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
            RelinkableHandle<Quote>(s2yRate), 
            2, Years, settlementDays, 
            calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
            RelinkableHandle<Quote>(s3yRate), 
            3, Years, settlementDays, 
            calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
            RelinkableHandle<Quote>(s5yRate), 
            5, Years, settlementDays, 
            calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(
            RelinkableHandle<Quote>(s10yRate), 
            10, Years, settlementDays, 
            calendar, ModifiedFollowing, swFixedLegFrequency,
            swFixedLegIsAdjusted, swFixedLegDayCounter,
            swFloatingLegFrequency));
        boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(
            RelinkableHandle<Quote>(s15yRate), 
            15, Years, settlementDays, 
            calendar, ModifiedFollowing, swFixedLegFrequency,
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
        std::vector<boost::shared_ptr<RateHelper> > depoSwapInstruments;
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
        boost::shared_ptr<TermStructure> depoSwapTermStructure(new
            PiecewiseFlatForward(todaysDate, settlementDate,
            depoSwapInstruments, termStructureDayCounter));


        // A depo-futures-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments;
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
        boost::shared_ptr<TermStructure> depoFutSwapTermStructure(new
            PiecewiseFlatForward(todaysDate, settlementDate,
            depoFutSwapInstruments, termStructureDayCounter));


        // A depo-FRA-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoFRASwapInstruments;
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
        boost::shared_ptr<TermStructure> depoFRASwapTermStructure(new
            PiecewiseFlatForward(todaysDate, settlementDate,
            depoFRASwapInstruments, termStructureDayCounter));


        // Term structures that will be used for pricing:
        // the one used for discounting cash flows
        RelinkableHandle<TermStructure> discountingTermStructure;
        // the one used for forward rate forecasting
        RelinkableHandle<TermStructure> forecastingTermStructure;


        /*********************
        * SWAPS TO BE PRICED *
        **********************/

        // constant nominal 1,000,000 Euro
        double nominal = 1000000.0;
        // fixed leg
        int fixedLegFrequency = 1; // annual
        bool fixedLegIsAdjusted = false;
        RollingConvention roll = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        int fixingDays = 2;
        Rate fixedRate = 0.04;

        // floating leg
        int floatingLegFrequency = 2;
        boost::shared_ptr<Xibor> euriborIndex(new Euribor(6, Months,
            forecastingTermStructure)); // using the forecasting curve
        Spread spread = 0.0;

        int lenghtInYears = 5;
        bool payFixedRate = true;
        SimpleSwap spot5YearSwap(payFixedRate, settlementDate, lenghtInYears,
            Years, calendar, roll, nominal, fixedLegFrequency, fixedRate,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, fixingDays, spread,
            discountingTermStructure); // using the discounting curve
        SimpleSwap oneYearForward5YearSwap(payFixedRate,
            calendar.advance(settlementDate, 1, Years, ModifiedFollowing),
            lenghtInYears, Years,
            calendar, roll, nominal, fixedLegFrequency, fixedRate,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            euriborIndex, fixingDays, spread,
            discountingTermStructure); // using the discounting curve


        /***************
        * SWAP PRICING *
        ****************/

        // utilities for reporting
        std::vector<std::string> headers(4);
        headers[0] = "term structure";
        headers[1] = "net present value";
        headers[2] = "fair spread";
        headers[3] = "fair fixed rate";
        std::string separator = " | ";
        Size width = headers[0].size() + separator.size() 
                   + headers[1].size() + separator.size() 
                   + headers[2].size() + separator.size()
                   + headers[3].size() + separator.size() - 1;
        std::string rule(width, '-'), dblrule(width, '=');
        std::string tab(8, ' ');

        // calculations

        std::cout << dblrule << std::endl;
        std::cout <<  "5-year market swap-rate = "
                  << RateFormatter::toString(s5yRate->value(),2) << std::endl;
        std::cout << dblrule << std::endl;

        std::cout << tab << "5-years swap paying "
                  << RateFormatter::toString(fixedRate,2) << std::endl;
        std::cout << headers[0] << separator
                  << headers[1] << separator
                  << headers[2] << separator
                  << headers[3] << separator << std::endl;
        std::cout << rule << std::endl;

        double NPV;
        Rate fairRate;
        Spread fairSpread;

        // Of course, you're not forced to really use different curves
        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(depoSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;


        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(QL_FABS(fairRate-s5yQuote)<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;

        QL_REQUIRE(QL_FABS(fairRate-s5yQuote)<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;

        QL_REQUIRE(QL_FABS(fairRate-s5yQuote)<1e-8,
                   "5-years swap mispriced!");


        std::cout << rule << std::endl;

        // now let's price the 1Y forward 5Y swap

        std::cout << tab << "5-years, 1-year forward swap paying "
                  << RateFormatter::toString(fixedRate,2) << std::endl;
        std::cout << headers[0] << separator
                  << headers[1] << separator
                  << headers[2] << separator
                  << headers[3] << separator << std::endl;
        std::cout << rule << std::endl;


        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(depoSwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;


        // now let's say that the 5-years swap rate goes up to 4.60%.
        // A smarter market element--say, connected to a data source-- would
        // notice the change itself. Since we're using SimpleQuotes,
        // we'll have to change the value manually--which forces us to
        // downcast the handle and use the SimpleQuote
        // interface. In any case, the point here is that a change in the
        // value contained in the Quote triggers a new bootstrapping
        // of the curve and a repricing of the swap.

        boost::shared_ptr<SimpleQuote> fiveYearsRate = 
            boost::dynamic_pointer_cast<SimpleQuote>(s5yRate);
        fiveYearsRate->setValue(0.0460);

        std::cout << dblrule << std::endl;
        std::cout <<  "5-year market swap-rate = "
                  << RateFormatter::toString(s5yRate->value(),2) << std::endl;
        std::cout << dblrule << std::endl;

        std::cout << tab << "5-years swap paying "
                  << RateFormatter::toString(fixedRate,2) << std::endl;
        std::cout << headers[0] << separator
                  << headers[1] << separator
                  << headers[2] << separator
                  << headers[3] << separator << std::endl;
        std::cout << rule << std::endl;

        // now get the updated results
        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(depoSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;

        QL_REQUIRE(QL_FABS(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;

        QL_REQUIRE(QL_FABS(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;

        QL_REQUIRE(QL_FABS(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");

        std::cout << rule << std::endl;

        // the 1Y forward 5Y swap changes as well

        std::cout << tab << "5-years, 1-year forward swap paying "
                  << RateFormatter::toString(fixedRate,2) << std::endl;
        std::cout << headers[0] << separator
                  << headers[1] << separator
                  << headers[2] << separator
                  << headers[3] << separator << std::endl;
        std::cout << rule << std::endl;


        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(depoSwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size()) 
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size()) 
                  << DoubleFormatter::toString(NPV,2) << separator;
        std::cout << std::setw(headers[2].size()) 
                  << RateFormatter::toString(fairSpread,4) << separator;
        std::cout << std::setw(headers[3].size()) 
                  << RateFormatter::toString(fairRate,4) << separator;
        std::cout << std::endl;

        return 0;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

