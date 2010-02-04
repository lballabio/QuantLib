/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano

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

/*  This example shows how to set up a Term Structure and then price a simple
    swap.
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

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();
        Date settlementDate(22, September, 2004);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);

        Integer fixingDays = 2;
        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
        // nothing to do with Date::todaysDate
        Settings::instance().evaluationDate() = todaysDate;


        todaysDate = Settings::instance().evaluationDate();
        std::cout << "Today: " << todaysDate.weekday()
                  << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
                  << ", " << settlementDate << std::endl;

        // deposits
        Rate d1wQuote=0.0382;
        Rate d1mQuote=0.0372;
        Rate d3mQuote=0.0363;
        Rate d6mQuote=0.0353;
        Rate d9mQuote=0.0348;
        Rate d1yQuote=0.0345;
        // FRAs
        Rate fra3x6Quote=0.037125;
        Rate fra6x9Quote=0.037125;
        Rate fra6x12Quote=0.037125;
        // futures
        Real fut1Quote=96.2875;
        Real fut2Quote=96.7875;
        Real fut3Quote=96.9875;
        Real fut4Quote=96.6875;
        Real fut5Quote=96.4875;
        Real fut6Quote=96.3875;
        Real fut7Quote=96.2875;
        Real fut8Quote=96.0875;
        // swaps
        Rate s2yQuote=0.037125;
        Rate s3yQuote=0.0398;
        Rate s5yQuote=0.0443;
        Rate s10yQuote=0.05165;
        Rate s15yQuote=0.055175;


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

        boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(
            Handle<Quote>(d1wRate),
            1*Weeks, fixingDays,
            calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(
            Handle<Quote>(d1mRate),
            1*Months, fixingDays,
            calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(
            Handle<Quote>(d3mRate),
            3*Months, fixingDays,
            calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> d6m(new DepositRateHelper(
            Handle<Quote>(d6mRate),
            6*Months, fixingDays,
            calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> d9m(new DepositRateHelper(
            Handle<Quote>(d9mRate),
            9*Months, fixingDays,
            calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> d1y(new DepositRateHelper(
            Handle<Quote>(d1yRate),
            1*Years, fixingDays,
            calendar, ModifiedFollowing,
            true, depositDayCounter));


        // setup FRAs
        boost::shared_ptr<RateHelper> fra3x6(new FraRateHelper(
            Handle<Quote>(fra3x6Rate),
            3, 6, fixingDays, calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x9(new FraRateHelper(
            Handle<Quote>(fra6x9Rate),
            6, 9, fixingDays, calendar, ModifiedFollowing,
            true, depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x12(new FraRateHelper(
            Handle<Quote>(fra6x12Rate),
            6, 12, fixingDays, calendar, ModifiedFollowing,
            true, depositDayCounter));


        // setup futures
        // Rate convexityAdjustment = 0.0;
        Integer futMonths = 3;
        Date imm = IMM::nextDate(settlementDate);
        boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(
            Handle<Quote>(fut2Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(
            Handle<Quote>(fut3Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(
            Handle<Quote>(fut4Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(
            Handle<Quote>(fut5Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(
            Handle<Quote>(fut6Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(
            Handle<Quote>(fut7Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));
        imm = IMM::nextDate(imm+1);
        boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(
            Handle<Quote>(fut8Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            true, depositDayCounter));


        // setup swaps
        Frequency swFixedLegFrequency = Annual;
        BusinessDayConvention swFixedLegConvention = Unadjusted;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
        boost::shared_ptr<IborIndex> swFloatingLegIndex(new Euribor6M);

        boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
            Handle<Quote>(s2yRate), 2*Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
            Handle<Quote>(s3yRate), 3*Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
            Handle<Quote>(s5yRate), 5*Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(
            Handle<Quote>(s10yRate), 10*Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(
            Handle<Quote>(s15yRate), 15*Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        DayCounter termStructureDayCounter =
            ActualActual(ActualActual::ISDA);


        double tolerance = 1.0e-15;

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
        boost::shared_ptr<YieldTermStructure> depoSwapTermStructure(
            new PiecewiseYieldCurve<Discount,LogLinear>(
                                          settlementDate, depoSwapInstruments,
                                          termStructureDayCounter,
                                          tolerance));


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
        boost::shared_ptr<YieldTermStructure> depoFutSwapTermStructure(
            new PiecewiseYieldCurve<Discount,LogLinear>(
                                       settlementDate, depoFutSwapInstruments,
                                       termStructureDayCounter,
                                       tolerance));


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
        boost::shared_ptr<YieldTermStructure> depoFRASwapTermStructure(
            new PiecewiseYieldCurve<Discount,LogLinear>(
                                       settlementDate, depoFRASwapInstruments,
                                       termStructureDayCounter,
                                       tolerance));


        // Term structures that will be used for pricing:
        // the one used for discounting cash flows
        RelinkableHandle<YieldTermStructure> discountingTermStructure;
        // the one used for forward rate forecasting
        RelinkableHandle<YieldTermStructure> forecastingTermStructure;


        /*********************
        * SWAPS TO BE PRICED *
        **********************/

        // constant nominal 1,000,000 Euro
        Real nominal = 1000000.0;
        // fixed leg
        Frequency fixedLegFrequency = Annual;
        BusinessDayConvention fixedLegConvention = Unadjusted;
        BusinessDayConvention floatingLegConvention = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        Rate fixedRate = 0.04;
        DayCounter floatingLegDayCounter = Actual360();

        // floating leg
        Frequency floatingLegFrequency = Semiannual;
        boost::shared_ptr<IborIndex> euriborIndex(
                                     new Euribor6M(forecastingTermStructure));
        Spread spread = 0.0;

        Integer lenghtInYears = 5;
        VanillaSwap::Type swapType = VanillaSwap::Payer;

        Date maturity = settlementDate + lenghtInYears*Years;
        Schedule fixedSchedule(settlementDate, maturity,
                               Period(fixedLegFrequency),
                               calendar, fixedLegConvention,
                               fixedLegConvention,
                               DateGeneration::Forward, false);
        Schedule floatSchedule(settlementDate, maturity,
                               Period(floatingLegFrequency),
                               calendar, floatingLegConvention,
                               floatingLegConvention,
                               DateGeneration::Forward, false);
        VanillaSwap spot5YearSwap(swapType, nominal,
            fixedSchedule, fixedRate, fixedLegDayCounter,
            floatSchedule, euriborIndex, spread,
            floatingLegDayCounter);

        Date fwdStart = calendar.advance(settlementDate, 1, Years);
        Date fwdMaturity = fwdStart + lenghtInYears*Years;
        Schedule fwdFixedSchedule(fwdStart, fwdMaturity,
                                  Period(fixedLegFrequency),
                                  calendar, fixedLegConvention,
                                  fixedLegConvention,
                                  DateGeneration::Forward, false);
        Schedule fwdFloatSchedule(fwdStart, fwdMaturity,
                                  Period(floatingLegFrequency),
                                  calendar, floatingLegConvention,
                                  floatingLegConvention,
                                  DateGeneration::Forward, false);
        VanillaSwap oneYearForward5YearSwap(swapType, nominal,
            fwdFixedSchedule, fixedRate, fixedLegDayCounter,
            fwdFloatSchedule, euriborIndex, spread,
            floatingLegDayCounter);


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
                  << std::setprecision(2) << io::rate(s5yRate->value())
                  << std::endl;
        std::cout << dblrule << std::endl;

        std::cout << tab << "5-years swap paying "
                  << io::rate(fixedRate) << std::endl;
        std::cout << headers[0] << separator
                  << headers[1] << separator
                  << headers[2] << separator
                  << headers[3] << separator << std::endl;
        std::cout << rule << std::endl;

        Real NPV;
        Rate fairRate;
        Spread fairSpread;

        boost::shared_ptr<PricingEngine> swapEngine(
                         new DiscountingSwapEngine(discountingTermStructure));

        spot5YearSwap.setPricingEngine(swapEngine);
        oneYearForward5YearSwap.setPricingEngine(swapEngine);

        // Of course, you're not forced to really use different curves
        forecastingTermStructure.linkTo(depoSwapTermStructure);
        discountingTermStructure.linkTo(depoSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;


        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(std::fabs(fairRate-s5yQuote)<1e-8,
                   "5-years swap mispriced by "
                   << io::rate(std::fabs(fairRate-s5yQuote)));


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;

        QL_REQUIRE(std::fabs(fairRate-s5yQuote)<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;

        QL_REQUIRE(std::fabs(fairRate-s5yQuote)<1e-8,
                   "5-years swap mispriced!");


        std::cout << rule << std::endl;

        // now let's price the 1Y forward 5Y swap

        std::cout << tab << "5-years, 1-year forward swap paying "
                  << io::rate(fixedRate) << std::endl;
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
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
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
                  << io::rate(s5yRate->value()) << std::endl;
        std::cout << dblrule << std::endl;

        std::cout << tab << "5-years swap paying "
                  << io::rate(fixedRate) << std::endl;
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
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;

        QL_REQUIRE(std::fabs(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;

        QL_REQUIRE(std::fabs(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;

        QL_REQUIRE(std::fabs(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");

        std::cout << rule << std::endl;

        // the 1Y forward 5Y swap changes as well

        std::cout << tab << "5-years, 1-year forward swap paying "
                  << io::rate(fixedRate) << std::endl;
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
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFutSwapTermStructure);
        discountingTermStructure.linkTo(depoFutSwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-fut-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;


        forecastingTermStructure.linkTo(depoFRASwapTermStructure);
        discountingTermStructure.linkTo(depoFRASwapTermStructure);

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << "depo-FRA-swap" << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate) << separator;
        std::cout << std::endl;

        Real seconds = timer.elapsed();
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

