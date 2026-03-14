/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2006 Allen Kuo

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

/*  This example shows how to set up a term structure and price a simple
    forward-rate agreement.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/daycounters/actualactual.hpp>

#include <iostream>

using namespace std;
using namespace QuantLib;

int main(int, char* []) {

    try {

        std::cout << std::endl;

        /*********************
         ***  MARKET DATA  ***
         *********************/

        RelinkableHandle<YieldTermStructure> euriborTermStructure;
        auto euribor3m = ext::make_shared<Euribor3M>(euriborTermStructure);

        Date todaysDate = Date(23, May, 2006);
        Settings::instance().evaluationDate() = todaysDate;

        Calendar calendar = euribor3m->fixingCalendar();
        Integer fixingDays = euribor3m->fixingDays();
        Date settlementDate = calendar.advance(todaysDate, fixingDays, Days);

        std::cout << "Today: " << todaysDate.weekday()
                  << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
                  << ", " << settlementDate << std::endl;


        // 3 month term FRA quotes (index refers to monthsToStart)
        Rate threeMonthFraQuote[10];

        threeMonthFraQuote[1]=0.030;
        threeMonthFraQuote[2]=0.031;
        threeMonthFraQuote[3]=0.032;
        threeMonthFraQuote[6]=0.033;
        threeMonthFraQuote[9]=0.034;

        /********************
         ***    QUOTES    ***
         ********************/

        // SimpleQuote stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.


        // FRAs
        auto fra1x4Rate = ext::make_shared<SimpleQuote>(threeMonthFraQuote[1]);
        auto fra2x5Rate = ext::make_shared<SimpleQuote>(threeMonthFraQuote[2]);
        auto fra3x6Rate = ext::make_shared<SimpleQuote>(threeMonthFraQuote[3]);
        auto fra6x9Rate = ext::make_shared<SimpleQuote>(threeMonthFraQuote[6]);
        auto fra9x12Rate = ext::make_shared<SimpleQuote>(threeMonthFraQuote[9]);

        RelinkableHandle<Quote> h1x4;  h1x4.linkTo(fra1x4Rate);
        RelinkableHandle<Quote> h2x5;  h2x5.linkTo(fra2x5Rate);
        RelinkableHandle<Quote> h3x6;  h3x6.linkTo(fra3x6Rate);
        RelinkableHandle<Quote> h6x9;  h6x9.linkTo(fra6x9Rate);
        RelinkableHandle<Quote> h9x12; h9x12.linkTo(fra9x12Rate);

        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument-dependent infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        DayCounter fraDayCounter = euribor3m->dayCounter();
        BusinessDayConvention convention = euribor3m->businessDayConvention();
        bool endOfMonth = euribor3m->endOfMonth();

        auto fra1x4 = ext::make_shared<FraRateHelper>(h1x4, 1, 4,
                                             fixingDays, calendar, convention,
                                             endOfMonth, fraDayCounter);

        auto fra2x5 = ext::make_shared<FraRateHelper>(h2x5, 2, 5,
                                             fixingDays, calendar, convention,
                                             endOfMonth, fraDayCounter);

        auto fra3x6 = ext::make_shared<FraRateHelper>(h3x6, 3, 6,
                                             fixingDays, calendar, convention,
                                             endOfMonth, fraDayCounter);

        auto fra6x9 = ext::make_shared<FraRateHelper>(h6x9, 6, 9,
                                             fixingDays, calendar, convention,
                                             endOfMonth, fraDayCounter);

        auto fra9x12 = ext::make_shared<FraRateHelper>(h9x12, 9, 12,
                                             fixingDays, calendar, convention,
                                             endOfMonth, fraDayCounter);


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        DayCounter termStructureDayCounter =
            ActualActual(ActualActual::ISDA);

        // A FRA curve
        std::vector<ext::shared_ptr<RateHelper>> fraInstruments;

        fraInstruments.push_back(fra1x4);
        fraInstruments.push_back(fra2x5);
        fraInstruments.push_back(fra3x6);
        fraInstruments.push_back(fra6x9);
        fraInstruments.push_back(fra9x12);

        auto fraTermStructure = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
                                         settlementDate, fraInstruments,
                                         termStructureDayCounter);

        /***********************
         ***  construct FRA's ***
         ***********************/

        Calendar fraCalendar = euribor3m->fixingCalendar();
        BusinessDayConvention fraBusinessDayConvention =
            euribor3m->businessDayConvention();
        Position::Type fraFwdType = Position::Long;
        Real fraNotional = 100.0;
        Integer monthsToStart[] = { 1, 2, 3, 6, 9 };

        euriborTermStructure.linkTo(fraTermStructure);

        cout << endl;
        cout << "Test FRA construction, NPV calculation, and FRA purchase"
             << endl
             << endl;

        Size i;
        for (i=0; i<std::size(monthsToStart); i++) {

            Date fraValueDate = fraCalendar.advance(
                                       settlementDate,monthsToStart[i],Months,
                                       fraBusinessDayConvention);

            Rate fraStrikeRate = threeMonthFraQuote[monthsToStart[i]];

            ForwardRateAgreement myFRA(euribor3m, fraValueDate,
                                       fraFwdType,fraStrikeRate,
                                       fraNotional);

            cout << "3m Term FRA, Months to Start: "
                 << monthsToStart[i]
                 << endl;
            cout << "strike FRA rate: "
                 << io::rate(fraStrikeRate)
                 << endl;
            cout << "FRA 3m forward rate: "
                 << myFRA.forwardRate()
                 << endl;
            cout << "FRA market quote: "
                 << io::rate(threeMonthFraQuote[monthsToStart[i]])
                 << endl;
            cout << "FRA amount [should be zero]: "
                 << myFRA.amount()
                 << endl;
            cout << "FRA NPV [should be zero]: "
                 << myFRA.NPV()
                 << endl
                 << endl;

        }




        cout << endl << endl;
        cout << "Now take a 100 basis-point upward shift in FRA quotes "
             << "and examine NPV"
             << endl
             << endl;

        const Real BpsShift = 0.01;

        threeMonthFraQuote[1]=0.030+BpsShift;
        threeMonthFraQuote[2]=0.031+BpsShift;
        threeMonthFraQuote[3]=0.032+BpsShift;
        threeMonthFraQuote[6]=0.033+BpsShift;
        threeMonthFraQuote[9]=0.034+BpsShift;

        fra1x4Rate->setValue(threeMonthFraQuote[1]);
        fra2x5Rate->setValue(threeMonthFraQuote[2]);
        fra3x6Rate->setValue(threeMonthFraQuote[3]);
        fra6x9Rate->setValue(threeMonthFraQuote[6]);
        fra9x12Rate->setValue(threeMonthFraQuote[9]);


        for (i=0; i<std::size(monthsToStart); i++) {

            Date fraValueDate = fraCalendar.advance(
                                       settlementDate,monthsToStart[i],Months,
                                       fraBusinessDayConvention);

            Rate fraStrikeRate =
                threeMonthFraQuote[monthsToStart[i]] - BpsShift;

            ForwardRateAgreement myFRA(euribor3m, fraValueDate,
                                       fraFwdType, fraStrikeRate,
                                       fraNotional);

            cout << "3m Term FRA, 100 notional, Months to Start = "
                 << monthsToStart[i]
                 << endl;
            cout << "strike FRA rate: "
                 << io::rate(fraStrikeRate)
                 << endl;
            cout << "FRA 3m forward rate: "
                 << myFRA.forwardRate()
                 << endl;
            cout << "FRA market quote: "
                 << io::rate(threeMonthFraQuote[monthsToStart[i]])
                 << endl;
            cout << "FRA amount [should be positive]: "
                 << myFRA.amount()
                 << endl;
            cout << "FRA NPV [should be positive]: "
                 << myFRA.NPV()
                 << endl
                 << endl;
        }

        return 0;

    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

