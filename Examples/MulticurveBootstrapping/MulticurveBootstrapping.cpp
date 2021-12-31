/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2018 Jose Garcia

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

/*  This example shows how to set up a term structure with OIS discounting
    and then price a simple 5 year swap.

    Example based on market data in paper by F. M. Ametrano and M. Bianchetti,
    Everything You Always Wanted to Know About Multiple Interest Rate Curve Boostrapping
    but Were Afraid to Ask (April 2, 2013).
    http://ssrn.com/abstract=2219548
    Eonia curve was taken from Figure 25 and Euribor 6m from figure 31.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/ibor/eonia.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/imm.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    ThreadKey sessionId() { return {}; }

}
#endif


int main(int, char* []) {

    try {

        std::cout << std::endl;

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();

        Date todaysDate(11, December, 2012);
        Settings::instance().evaluationDate() = todaysDate;
        todaysDate = Settings::instance().evaluationDate();

        Integer fixingDays = 2;
        Date settlementDate = calendar.advance(todaysDate, fixingDays, Days);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);


        std::cout << "Today: " << todaysDate.weekday()
                  << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
                  << ", " << settlementDate << std::endl;

        /********************
         ***    QUOTES    ***
         ********************/

        // SimpleQuote stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.

        // deposits
        ext::shared_ptr<Quote> dONRate(new SimpleQuote(0.0004));
        ext::shared_ptr<Quote> dTNRate(new SimpleQuote(0.0004));
        ext::shared_ptr<Quote> dSNRate(new SimpleQuote(0.0004));

        // OIS
        ext::shared_ptr<Quote> ois1WRate(new SimpleQuote(0.00070));
        ext::shared_ptr<Quote> ois2WRate(new SimpleQuote(0.00069));
        ext::shared_ptr<Quote> ois3WRate(new SimpleQuote(0.00078));
        ext::shared_ptr<Quote> ois1MRate(new SimpleQuote(0.00074));

        // Dated OIS
        ext::shared_ptr<Quote> oisDated1Rate(new SimpleQuote( 0.000460));
        ext::shared_ptr<Quote> oisDated2Rate(new SimpleQuote( 0.000160));
        ext::shared_ptr<Quote> oisDated3Rate(new SimpleQuote(-0.000070));
        ext::shared_ptr<Quote> oisDated4Rate(new SimpleQuote(-0.000130));
        ext::shared_ptr<Quote> oisDated5Rate(new SimpleQuote(-0.000140));

        // OIS
        ext::shared_ptr<Quote> ois15MRate(new SimpleQuote(0.00002));
        ext::shared_ptr<Quote> ois18MRate(new SimpleQuote(0.00008));
        ext::shared_ptr<Quote> ois21MRate(new SimpleQuote(0.00021));
        ext::shared_ptr<Quote> ois2YRate(new SimpleQuote(0.00036));
        ext::shared_ptr<Quote> ois3YRate(new SimpleQuote(0.00127));
        ext::shared_ptr<Quote> ois4YRate(new SimpleQuote(0.00274));
        ext::shared_ptr<Quote> ois5YRate(new SimpleQuote(0.00456));
        ext::shared_ptr<Quote> ois6YRate(new SimpleQuote(0.00647));
        ext::shared_ptr<Quote> ois7YRate(new SimpleQuote(0.00827));
        ext::shared_ptr<Quote> ois8YRate(new SimpleQuote(0.00996));
        ext::shared_ptr<Quote> ois9YRate(new SimpleQuote(0.01147));
        ext::shared_ptr<Quote> ois10YRate(new SimpleQuote(0.0128));
        ext::shared_ptr<Quote> ois11YRate(new SimpleQuote(0.01404));
        ext::shared_ptr<Quote> ois12YRate(new SimpleQuote(0.01516));
        ext::shared_ptr<Quote> ois15YRate(new SimpleQuote(0.01764));
        ext::shared_ptr<Quote> ois20YRate(new SimpleQuote(0.01939));
        ext::shared_ptr<Quote> ois25YRate(new SimpleQuote(0.02003));
        ext::shared_ptr<Quote> ois30YRate(new SimpleQuote(0.02038));

        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // deposits
        DayCounter depositDayCounter = Actual360();

        ext::shared_ptr<RateHelper> dON(new DepositRateHelper(
            Handle<Quote>(dONRate),
            1 * Days, 0,
            calendar, Following,
            false, depositDayCounter));
        ext::shared_ptr<RateHelper> dTN(new DepositRateHelper(
            Handle<Quote>(dTNRate),
            1 * Days, 1,
            calendar, Following,
            false, depositDayCounter));
        ext::shared_ptr<RateHelper> dSN(new DepositRateHelper(
            Handle<Quote>(dSNRate),
            1 * Days, 2,
            calendar, Following,
            false, depositDayCounter));

        // OIS

        ext::shared_ptr<Eonia> eonia(new Eonia);

        ext::shared_ptr<RateHelper> ois1W(new OISRateHelper(
            2, 1 * Weeks,
            Handle<Quote>(ois1WRate), eonia));
        ext::shared_ptr<RateHelper> ois2W(new OISRateHelper(
            2, 2 * Weeks,
            Handle<Quote>(ois2WRate), eonia));
        ext::shared_ptr<RateHelper> ois3W(new OISRateHelper(
            2, 3 * Weeks,
            Handle<Quote>(ois3WRate), eonia));
        ext::shared_ptr<RateHelper> ois1M(new OISRateHelper(
            2, 1 * Months,
            Handle<Quote>(ois1MRate), eonia));


        // Dated OIS

        ext::shared_ptr<RateHelper> oisDated1(new DatedOISRateHelper(
            Date(16, January, 2013), Date(13, February, 2013),
            Handle<Quote>(oisDated1Rate), eonia));
        ext::shared_ptr<RateHelper> oisDated2(new DatedOISRateHelper(
            Date(13, February, 2013), Date(13, March, 2013),
            Handle<Quote>(oisDated2Rate), eonia));
        ext::shared_ptr<RateHelper> oisDated3(new DatedOISRateHelper(
            Date(13, March, 2013), Date(10, April, 2013),
            Handle<Quote>(oisDated3Rate), eonia));
        ext::shared_ptr<RateHelper> oisDated4(new DatedOISRateHelper(
            Date(10, April, 2013), Date(8, May, 2013),
            Handle<Quote>(oisDated4Rate), eonia));
        ext::shared_ptr<RateHelper> oisDated5(new DatedOISRateHelper(
            Date(8, May, 2013), Date(12, June, 2013),
            Handle<Quote>(oisDated5Rate), eonia));

        // OIS
        ext::shared_ptr<RateHelper> ois15M(new OISRateHelper(
            2, 15*Months,
            Handle<Quote>(ois15MRate), eonia));
        ext::shared_ptr<RateHelper> ois18M(new OISRateHelper(
            2, 18*Months,
            Handle<Quote>(ois18MRate), eonia));
        ext::shared_ptr<RateHelper> ois21M(new OISRateHelper(
            2, 21*Months,
            Handle<Quote>(ois21MRate), eonia));
        ext::shared_ptr<RateHelper> ois2Y(new OISRateHelper(
            2, 2*Years,
            Handle<Quote>(ois2YRate), eonia));
        ext::shared_ptr<RateHelper> ois3Y(new OISRateHelper(
            2, 3*Years,
            Handle<Quote>(ois3YRate), eonia));
        ext::shared_ptr<RateHelper> ois4Y(new OISRateHelper(
            2, 4*Years,
            Handle<Quote>(ois4YRate), eonia));
        ext::shared_ptr<RateHelper> ois5Y(new OISRateHelper(
            2, 5*Years,
            Handle<Quote>(ois5YRate), eonia));
        ext::shared_ptr<RateHelper> ois6Y(new OISRateHelper(
            2, 6*Years,
            Handle<Quote>(ois6YRate), eonia));
        ext::shared_ptr<RateHelper> ois7Y(new OISRateHelper(
            2, 7*Years,
            Handle<Quote>(ois7YRate), eonia));
        ext::shared_ptr<RateHelper> ois8Y(new OISRateHelper(
            2, 8*Years,
            Handle<Quote>(ois8YRate), eonia));
        ext::shared_ptr<RateHelper> ois9Y(new OISRateHelper(
            2, 9*Years,
            Handle<Quote>(ois9YRate), eonia));
        ext::shared_ptr<RateHelper> ois10Y(new OISRateHelper(
            2, 10*Years,
            Handle<Quote>(ois10YRate), eonia));
        ext::shared_ptr<RateHelper> ois11Y(new OISRateHelper(
            2, 11*Years,
            Handle<Quote>(ois11YRate), eonia));
        ext::shared_ptr<RateHelper> ois12Y(new OISRateHelper(
            2, 12*Years,
            Handle<Quote>(ois12YRate), eonia));
        ext::shared_ptr<RateHelper> ois15Y(new OISRateHelper(
            2, 15*Years,
            Handle<Quote>(ois15YRate), eonia));
        ext::shared_ptr<RateHelper> ois20Y(new OISRateHelper(
            2, 20*Years,
            Handle<Quote>(ois20YRate), eonia));
        ext::shared_ptr<RateHelper> ois25Y(new OISRateHelper(
            2, 25*Years,
            Handle<Quote>(ois25YRate), eonia));
        ext::shared_ptr<RateHelper> ois30Y(new OISRateHelper(
            2, 30*Years,
            Handle<Quote>(ois30YRate), eonia));


        /*********************
        **  CURVE BUILDING **
        *********************/

        /*********************
        **   EONIA CURVE    **
        *********************/

        DayCounter termStructureDayCounter = Actual365Fixed();

        // Eonia curve
        std::vector<ext::shared_ptr<RateHelper> > eoniaInstruments;
        eoniaInstruments.push_back(dON);
        eoniaInstruments.push_back(dTN);
        eoniaInstruments.push_back(dSN);
        eoniaInstruments.push_back(ois1W);
        eoniaInstruments.push_back(ois2W);
        eoniaInstruments.push_back(ois3W);
        eoniaInstruments.push_back(ois1M);
        eoniaInstruments.push_back(oisDated1);
        eoniaInstruments.push_back(oisDated2);
        eoniaInstruments.push_back(oisDated3);
        eoniaInstruments.push_back(oisDated4);
        eoniaInstruments.push_back(oisDated5);
        eoniaInstruments.push_back(ois15M);
        eoniaInstruments.push_back(ois18M);
        eoniaInstruments.push_back(ois21M);
        eoniaInstruments.push_back(ois2Y);
        eoniaInstruments.push_back(ois3Y);
        eoniaInstruments.push_back(ois4Y);
        eoniaInstruments.push_back(ois5Y);
        eoniaInstruments.push_back(ois6Y);
        eoniaInstruments.push_back(ois7Y);
        eoniaInstruments.push_back(ois8Y);
        eoniaInstruments.push_back(ois9Y);
        eoniaInstruments.push_back(ois10Y);
        eoniaInstruments.push_back(ois11Y);
        eoniaInstruments.push_back(ois12Y);
        eoniaInstruments.push_back(ois15Y);
        eoniaInstruments.push_back(ois20Y);
        eoniaInstruments.push_back(ois25Y);
        eoniaInstruments.push_back(ois30Y);

        ext::shared_ptr<YieldTermStructure> eoniaTermStructure(
            new PiecewiseYieldCurve<Discount, Cubic>(
                todaysDate, eoniaInstruments,
                termStructureDayCounter) );

        eoniaTermStructure->enableExtrapolation();


        // Term structures that will be used for pricing:
        // the one used for discounting cash flows
        RelinkableHandle<YieldTermStructure> discountingTermStructure;
        // the one used for forward rate forecasting
        RelinkableHandle<YieldTermStructure> forecastingTermStructure;

        discountingTermStructure.linkTo(eoniaTermStructure);


        /*********************
        **    EURIBOR 6M    **
        *********************/
        ext::shared_ptr<IborIndex> euribor6M(new Euribor6M);

        // deposits
        ext::shared_ptr<Quote> d6MRate(new SimpleQuote(0.00312));

        // FRAs
        ext::shared_ptr<Quote> fra1Rate(new SimpleQuote(0.002930));
        ext::shared_ptr<Quote> fra2Rate(new SimpleQuote(0.002720));
        ext::shared_ptr<Quote> fra3Rate(new SimpleQuote(0.002600));
        ext::shared_ptr<Quote> fra4Rate(new SimpleQuote(0.002560));
        ext::shared_ptr<Quote> fra5Rate(new SimpleQuote(0.002520));
        ext::shared_ptr<Quote> fra6Rate(new SimpleQuote(0.002480));
        ext::shared_ptr<Quote> fra7Rate(new SimpleQuote(0.002540));
        ext::shared_ptr<Quote> fra8Rate(new SimpleQuote(0.002610));
        ext::shared_ptr<Quote> fra9Rate(new SimpleQuote(0.002670));
        ext::shared_ptr<Quote> fra10Rate(new SimpleQuote(0.002790));
        ext::shared_ptr<Quote> fra11Rate(new SimpleQuote(0.002910));
        ext::shared_ptr<Quote> fra12Rate(new SimpleQuote(0.003030));
        ext::shared_ptr<Quote> fra13Rate(new SimpleQuote(0.003180));
        ext::shared_ptr<Quote> fra14Rate(new SimpleQuote(0.003350));
        ext::shared_ptr<Quote> fra15Rate(new SimpleQuote(0.003520));
        ext::shared_ptr<Quote> fra16Rate(new SimpleQuote(0.003710));
        ext::shared_ptr<Quote> fra17Rate(new SimpleQuote(0.003890));
        ext::shared_ptr<Quote> fra18Rate(new SimpleQuote(0.004090));

        //swaps
        ext::shared_ptr<Quote> s3yRate(new SimpleQuote(0.004240));
        ext::shared_ptr<Quote> s4yRate(new SimpleQuote(0.005760));
        ext::shared_ptr<Quote> s5yRate(new SimpleQuote(0.007620));
        ext::shared_ptr<Quote> s6yRate(new SimpleQuote(0.009540));
        ext::shared_ptr<Quote> s7yRate(new SimpleQuote(0.011350));
        ext::shared_ptr<Quote> s8yRate(new SimpleQuote(0.013030));
        ext::shared_ptr<Quote> s9yRate(new SimpleQuote(0.014520));
        ext::shared_ptr<Quote> s10yRate(new SimpleQuote(0.015840));
        ext::shared_ptr<Quote> s12yRate(new SimpleQuote(0.018090));
        ext::shared_ptr<Quote> s15yRate(new SimpleQuote(0.020370));
        ext::shared_ptr<Quote> s20yRate(new SimpleQuote(0.021870));
        ext::shared_ptr<Quote> s25yRate(new SimpleQuote(0.022340));
        ext::shared_ptr<Quote> s30yRate(new SimpleQuote(0.022560));
        ext::shared_ptr<Quote> s35yRate(new SimpleQuote(0.022950));
        ext::shared_ptr<Quote> s40yRate(new SimpleQuote(0.023480));
        ext::shared_ptr<Quote> s50yRate(new SimpleQuote(0.024210));
        ext::shared_ptr<Quote> s60yRate(new SimpleQuote(0.024630));


        ext::shared_ptr<RateHelper> d6M(new DepositRateHelper(
            Handle<Quote>(d6MRate),
            6 * Months, 3,
            calendar, Following,
            false, depositDayCounter));

        ext::shared_ptr<RateHelper> fra1(new FraRateHelper(
            Handle<Quote>(fra1Rate),
            1, euribor6M));
        ext::shared_ptr<RateHelper> fra2(new FraRateHelper(
            Handle<Quote>(fra2Rate),
            2, euribor6M));
        ext::shared_ptr<RateHelper> fra3(new FraRateHelper(
            Handle<Quote>(fra3Rate),
            3, euribor6M));
        ext::shared_ptr<RateHelper> fra4(new FraRateHelper(
            Handle<Quote>(fra4Rate),
            4, euribor6M));
        ext::shared_ptr<RateHelper> fra5(new FraRateHelper(
            Handle<Quote>(fra5Rate),
            5, euribor6M));
        ext::shared_ptr<RateHelper> fra6(new FraRateHelper(
            Handle<Quote>(fra6Rate),
            6, euribor6M));
        ext::shared_ptr<RateHelper> fra7(new FraRateHelper(
            Handle<Quote>(fra7Rate),
            7, euribor6M));
        ext::shared_ptr<RateHelper> fra8(new FraRateHelper(
            Handle<Quote>(fra8Rate),
            8, euribor6M));
        ext::shared_ptr<RateHelper> fra9(new FraRateHelper(
            Handle<Quote>(fra9Rate),
            9, euribor6M));
        ext::shared_ptr<RateHelper> fra10(new FraRateHelper(
            Handle<Quote>(fra10Rate),
            10, euribor6M));
        ext::shared_ptr<RateHelper> fra11(new FraRateHelper(
            Handle<Quote>(fra11Rate),
            11, euribor6M));
        ext::shared_ptr<RateHelper> fra12(new FraRateHelper(
            Handle<Quote>(fra12Rate),
            12, euribor6M));
        ext::shared_ptr<RateHelper> fra13(new FraRateHelper(
            Handle<Quote>(fra13Rate),
            13, euribor6M));
        ext::shared_ptr<RateHelper> fra14(new FraRateHelper(
            Handle<Quote>(fra14Rate),
            14, euribor6M));
        ext::shared_ptr<RateHelper> fra15(new FraRateHelper(
            Handle<Quote>(fra15Rate),
            15, euribor6M));
        ext::shared_ptr<RateHelper> fra16(new FraRateHelper(
            Handle<Quote>(fra16Rate),
            16, euribor6M));
        ext::shared_ptr<RateHelper> fra17(new FraRateHelper(
            Handle<Quote>(fra17Rate),
            17, euribor6M));
        ext::shared_ptr<RateHelper> fra18(new FraRateHelper(
            Handle<Quote>(fra18Rate),
            18, euribor6M));


        // setup swaps

        Frequency swFixedLegFrequency = Annual;
        BusinessDayConvention swFixedLegConvention = Unadjusted;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);

        ext::shared_ptr<IborIndex> swFloatingLegIndex(new Euribor6M);

        ext::shared_ptr<RateHelper> s3y(new SwapRateHelper(
            Handle<Quote>(s3yRate), 3 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s4y(new SwapRateHelper(
            Handle<Quote>(s4yRate), 4 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s5y(new SwapRateHelper(
            Handle<Quote>(s5yRate), 5 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s6y(new SwapRateHelper(
            Handle<Quote>(s6yRate), 6 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s7y(new SwapRateHelper(
            Handle<Quote>(s7yRate), 7 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s8y(new SwapRateHelper(
            Handle<Quote>(s8yRate), 8 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s9y(new SwapRateHelper(
            Handle<Quote>(s9yRate), 9 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s10y(new SwapRateHelper(
            Handle<Quote>(s10yRate), 10 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s12y(new SwapRateHelper(
            Handle<Quote>(s12yRate), 12 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s15y(new SwapRateHelper(
            Handle<Quote>(s15yRate), 15 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s20y(new SwapRateHelper(
            Handle<Quote>(s20yRate), 20 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s25y(new SwapRateHelper(
            Handle<Quote>(s25yRate), 25 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s30y(new SwapRateHelper(
            Handle<Quote>(s30yRate), 30 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s35y(new SwapRateHelper(
            Handle<Quote>(s35yRate), 35 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s40y(new SwapRateHelper(
            Handle<Quote>(s40yRate), 40 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s50y(new SwapRateHelper(
            Handle<Quote>(s50yRate), 50 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        ext::shared_ptr<RateHelper> s60y(new SwapRateHelper(
            Handle<Quote>(s60yRate), 60 * Years,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex,
            Handle<Quote>(), 0 * Days, discountingTermStructure));

        // Euribor 6M curve
        std::vector<ext::shared_ptr<RateHelper> > euribor6MInstruments;
        euribor6MInstruments.push_back(d6M);
        euribor6MInstruments.push_back(fra1);
        euribor6MInstruments.push_back(fra2);
        euribor6MInstruments.push_back(fra3);
        euribor6MInstruments.push_back(fra4);
        euribor6MInstruments.push_back(fra5);
        euribor6MInstruments.push_back(fra6);
        euribor6MInstruments.push_back(fra7);
        euribor6MInstruments.push_back(fra8);
        euribor6MInstruments.push_back(fra9);
        euribor6MInstruments.push_back(fra10);
        euribor6MInstruments.push_back(fra11);
        euribor6MInstruments.push_back(fra12);
        euribor6MInstruments.push_back(fra13);
        euribor6MInstruments.push_back(fra14);
        euribor6MInstruments.push_back(fra15);
        euribor6MInstruments.push_back(fra16);
        euribor6MInstruments.push_back(fra17);
        euribor6MInstruments.push_back(fra18);
        euribor6MInstruments.push_back(s3y);
        euribor6MInstruments.push_back(s4y);
        euribor6MInstruments.push_back(s5y);
        euribor6MInstruments.push_back(s6y);
        euribor6MInstruments.push_back(s7y);
        euribor6MInstruments.push_back(s8y);
        euribor6MInstruments.push_back(s9y);
        euribor6MInstruments.push_back(s10y);
        euribor6MInstruments.push_back(s12y);
        euribor6MInstruments.push_back(s15y);
        euribor6MInstruments.push_back(s20y);
        euribor6MInstruments.push_back(s25y);
        euribor6MInstruments.push_back(s30y);
        euribor6MInstruments.push_back(s35y);
        euribor6MInstruments.push_back(s40y);
        euribor6MInstruments.push_back(s50y);
        euribor6MInstruments.push_back(s60y);


        // If needed, it's possible to change the tolerance; the default is 1.0e-12.
        double tolerance = 1.0e-15;

        // The tolerance is passed in an explicit bootstrap object. Depending on
        // the bootstrap algorithm, it's possible to pass other parameters.
        ext::shared_ptr<YieldTermStructure> euribor6MTermStructure(
            new PiecewiseYieldCurve<Discount, Cubic>(
                settlementDate, euribor6MInstruments,
                termStructureDayCounter,
                PiecewiseYieldCurve<Discount, Cubic>::bootstrap_type(tolerance)));

        forecastingTermStructure.linkTo(euribor6MTermStructure);

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
        Rate fixedRate = 0.007;
        DayCounter floatingLegDayCounter = Actual360();

        // floating leg
        Frequency floatingLegFrequency = Semiannual;
        ext::shared_ptr<IborIndex> euriborIndex(
                                     new Euribor6M(forecastingTermStructure));
        Spread spread = 0.0;

        Integer lengthInYears = 5;
        Swap::Type swapType = Swap::Payer;

        Date maturity = settlementDate + lengthInYears*Years;
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
        Date fwdMaturity = fwdStart + lengthInYears*Years;
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

        // utilities for formatting the report

        std::ostringstream s1;
        s1 << "5-years swap paying " << std::setprecision(2) << io::rate(fixedRate);
        std::string case1 = s1.str();

        std::ostringstream s2;
        s2 << "5-years, 1-year forward swap paying " << std::setprecision(2) << io::rate(fixedRate);
        std::string case2 = s2.str();

        std::vector<std::string> headers(4);
        headers[0] = std::string(std::max(case1.size(), case2.size()) + 1, ' ');
        headers[1] = "net present value";
        headers[2] = "fair spread";
        headers[3] = "fair fixed rate";
        std::string separator = " | ";
        std::string header = headers[0] + separator + headers[1] + separator + headers[2] + separator + headers[3];
        Size width = header.size();
        std::string rule(width, '-'), dblrule(width, '=');

        // calculations

        std::cout << dblrule << std::endl;
        std::cout << " With 5-year market swap-rate = "
                  << std::setprecision(2) << io::rate(s5yRate->value())
                  << std::endl;
        std::cout << rule << std::endl;

        std::cout << header << std::endl;
        std::cout << rule << std::endl;

        Real NPV;
        Rate fairRate;
        Spread fairSpread;

        ext::shared_ptr<PricingEngine> swapEngine(
                         new DiscountingSwapEngine(discountingTermStructure));

        spot5YearSwap.setPricingEngine(swapEngine);
        oneYearForward5YearSwap.setPricingEngine(swapEngine);

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << case1 << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate);
        std::cout << std::endl;

        std::cout << rule << std::endl;

        // let's check that the 5 years swap has been correctly re-priced
        QL_REQUIRE(std::fabs(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced by "
                   << io::rate(std::fabs(fairRate-s5yRate->value())));

        // now let's price the 1Y forward 5Y swap

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << case2 << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate);
        std::cout << std::endl;

        // now let's say that the 5-years swap rate goes up to 0.009%.
        // A smarter market element--say, connected to a data source-- would
        // notice the change itself. Since we're using SimpleQuotes,
        // we'll have to change the value manually--which forces us to
        // downcast the handle and use the SimpleQuote
        // interface. In any case, the point here is that a change in the
        // value contained in the Quote triggers a new bootstrapping
        // of the curve and a repricing of the swap.

        ext::shared_ptr<SimpleQuote> fiveYearsRate =
            ext::dynamic_pointer_cast<SimpleQuote>(s5yRate);
        fiveYearsRate->setValue(0.0090);

        std::cout << dblrule << std::endl;
        std::cout << " With 5-year market swap-rate = "
                  << io::rate(s5yRate->value()) << std::endl;
        std::cout << rule << std::endl;

        std::cout << header << std::endl;
        std::cout << rule << std::endl;

        // now get the updated results

        NPV = spot5YearSwap.NPV();
        fairSpread = spot5YearSwap.fairSpread();
        fairRate = spot5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << case1 << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate);
        std::cout << std::endl;

        QL_REQUIRE(std::fabs(fairRate-s5yRate->value())<1e-8,
                   "5-years swap mispriced!");

        std::cout << rule << std::endl;

        // the 1Y forward 5Y swap doesn't change;
        // it depends on the 1-year and 6-years rates, which didn't move

        NPV = oneYearForward5YearSwap.NPV();
        fairSpread = oneYearForward5YearSwap.fairSpread();
        fairRate = oneYearForward5YearSwap.fairRate();

        std::cout << std::setw(headers[0].size())
                  << case2 << separator;
        std::cout << std::setw(headers[1].size())
                  << std::fixed << std::setprecision(2) << NPV << separator;
        std::cout << std::setw(headers[2].size())
                  << io::rate(fairSpread) << separator;
        std::cout << std::setw(headers[3].size())
                  << io::rate(fairRate);
        std::cout << std::endl;

        std::cout << dblrule << std::endl;

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
