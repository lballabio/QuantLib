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

        /*************************
         ***  GLOBAL SETTINGS  ***
         *************************/

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

        /*********************
        **   EONIA CURVE    **
        *********************/

        DayCounter termStructureDayCounter = Actual365Fixed();
        std::vector<ext::shared_ptr<RateHelper> > eoniaInstruments;

        ext::shared_ptr<Eonia> eonia(new Eonia);

        // a SimpleQuote instance stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.

        // RateHelpers are built from the quotes, together with
        // other info depending on the instrument.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // deposits

        std::map<Natural, ext::shared_ptr<Quote>> depoQuotes = {
            // settlement days, quote
            {0, ext::make_shared<SimpleQuote>(0.0004)},
            {1, ext::make_shared<SimpleQuote>(0.0004)},
            {2, ext::make_shared<SimpleQuote>(0.0004)}
        };

        DayCounter depositDayCounter = Actual360();

        for (auto q : depoQuotes) {
            auto settlementDays = q.first;
            auto quote = q.second;
            auto helper = ext::make_shared<DepositRateHelper>(
                Handle<Quote>(quote),
                1 * Days, settlementDays,
                calendar, Following,
                false, depositDayCounter);
            eoniaInstruments.push_back(helper);
        }

        // short-term OIS

        std::map<Period, ext::shared_ptr<Quote>> shortOisQuotes = {
            {1 * Weeks, ext::make_shared<SimpleQuote>(0.00070)},
            {2 * Weeks, ext::make_shared<SimpleQuote>(0.00069)},
            {3 * Weeks, ext::make_shared<SimpleQuote>(0.00078)},
            {1 * Months, ext::make_shared<SimpleQuote>(0.00074)}
        };

        for (auto q : shortOisQuotes) {
            auto tenor = q.first;
            auto quote = q.second;
            auto helper = ext::make_shared<OISRateHelper>(
                2, tenor, Handle<Quote>(quote), eonia);
            eoniaInstruments.push_back(helper);
        }

        // Dated OIS

        std::map<std::pair<Date, Date>, ext::shared_ptr<Quote>> datedOisQuotes = {
            {{Date(16, January, 2013), Date(13, February, 2013)}, ext::make_shared<SimpleQuote>( 0.000460)},
            {{Date(13, February, 2013), Date(13, March, 2013)}, ext::make_shared<SimpleQuote>( 0.000160)},
            {{Date(13, March, 2013), Date(10, April, 2013)}, ext::make_shared<SimpleQuote>(-0.000070)},
            {{Date(10, April, 2013), Date(8, May, 2013)}, ext::make_shared<SimpleQuote>(-0.000130)},
            {{Date(8, May, 2013), Date(12, June, 2013)}, ext::make_shared<SimpleQuote>(-0.000140)},
        };

        for (auto q : datedOisQuotes) {
            auto startDate = q.first.first;
            auto endDate = q.first.second;
            auto quote = q.second;
            auto helper = ext::make_shared<DatedOISRateHelper>(
                startDate, endDate, Handle<Quote>(quote), eonia);
            eoniaInstruments.push_back(helper);
        }

        // long-term OIS

        std::map<Period, ext::shared_ptr<Quote>> longOisQuotes = {
            {15 * Months, ext::make_shared<SimpleQuote>(0.00002)},
            {18 * Months, ext::make_shared<SimpleQuote>(0.00008)},
            {21 * Months, ext::make_shared<SimpleQuote>(0.00021)},
            {2 * Years, ext::make_shared<SimpleQuote>(0.00036)},
            {3 * Years, ext::make_shared<SimpleQuote>(0.00127)},
            {4 * Years, ext::make_shared<SimpleQuote>(0.00274)},
            {5 * Years, ext::make_shared<SimpleQuote>(0.00456)},
            {6 * Years, ext::make_shared<SimpleQuote>(0.00647)},
            {7 * Years, ext::make_shared<SimpleQuote>(0.00827)},
            {8 * Years, ext::make_shared<SimpleQuote>(0.00996)},
            {9 * Years, ext::make_shared<SimpleQuote>(0.01147)},
            {10 * Years, ext::make_shared<SimpleQuote>(0.0128)},
            {11 * Years, ext::make_shared<SimpleQuote>(0.01404)},
            {12 * Years, ext::make_shared<SimpleQuote>(0.01516)},
            {15 * Years, ext::make_shared<SimpleQuote>(0.01764)},
            {20 * Years, ext::make_shared<SimpleQuote>(0.01939)},
            {25 * Years, ext::make_shared<SimpleQuote>(0.02003)},
            {30 * Years, ext::make_shared<SimpleQuote>(0.02038)}
        };

        for (auto q : longOisQuotes) {
            auto tenor = q.first;
            auto quote = q.second;
            auto helper = ext::make_shared<OISRateHelper>(
                2, tenor, Handle<Quote>(quote), eonia);
            eoniaInstruments.push_back(helper);
        }

        // curve

        ext::shared_ptr<YieldTermStructure> eoniaTermStructure(
            new PiecewiseYieldCurve<Discount, Cubic>(
                todaysDate, eoniaInstruments,
                termStructureDayCounter) );

        eoniaTermStructure->enableExtrapolation();

        // This curve will be used for discounting cash flows
        RelinkableHandle<YieldTermStructure> discountingTermStructure;
        discountingTermStructure.linkTo(eoniaTermStructure);


        /**************************
        **    EURIBOR 6M CURVE   **
        ***************************/

        std::vector<ext::shared_ptr<RateHelper> > euribor6MInstruments;

        ext::shared_ptr<IborIndex> euribor6M(new Euribor6M);

        // deposits

        auto d6MRate = ext::make_shared<SimpleQuote>(0.00312);

        auto d6M = ext::make_shared<DepositRateHelper>(
            Handle<Quote>(d6MRate),
            6 * Months, 3,
            calendar, Following,
            false, depositDayCounter);

        euribor6MInstruments.push_back(d6M);

        // FRAs

        std::map<Natural, ext::shared_ptr<Quote>> fraQuotes = {
            {1, ext::make_shared<SimpleQuote>(0.002930)},
            {2, ext::make_shared<SimpleQuote>(0.002720)},
            {3, ext::make_shared<SimpleQuote>(0.002600)},
            {4, ext::make_shared<SimpleQuote>(0.002560)},
            {5, ext::make_shared<SimpleQuote>(0.002520)},
            {6, ext::make_shared<SimpleQuote>(0.002480)},
            {7, ext::make_shared<SimpleQuote>(0.002540)},
            {8, ext::make_shared<SimpleQuote>(0.002610)},
            {9, ext::make_shared<SimpleQuote>(0.002670)},
            {10, ext::make_shared<SimpleQuote>(0.002790)},
            {11, ext::make_shared<SimpleQuote>(0.002910)},
            {12, ext::make_shared<SimpleQuote>(0.003030)},
            {13, ext::make_shared<SimpleQuote>(0.003180)},
            {14, ext::make_shared<SimpleQuote>(0.003350)},
            {15, ext::make_shared<SimpleQuote>(0.003520)},
            {16, ext::make_shared<SimpleQuote>(0.003710)},
            {17, ext::make_shared<SimpleQuote>(0.003890)},
            {18, ext::make_shared<SimpleQuote>(0.004090)}
        };

        for (auto q: fraQuotes) {
            auto monthsToStart = q.first;
            auto quote = q.second;
            auto helper = ext::make_shared<FraRateHelper>(
                Handle<Quote>(quote),
                monthsToStart, euribor6M);
            euribor6MInstruments.push_back(helper);
        }

        // swaps

        std::map<Period, ext::shared_ptr<Quote>> swapQuotes = {
            {3 * Years, ext::make_shared<SimpleQuote>(0.004240)},
            {4 * Years, ext::make_shared<SimpleQuote>(0.005760)},
            {5 * Years, ext::make_shared<SimpleQuote>(0.007620)},
            {6 * Years, ext::make_shared<SimpleQuote>(0.009540)},
            {7 * Years, ext::make_shared<SimpleQuote>(0.011350)},
            {8 * Years, ext::make_shared<SimpleQuote>(0.013030)},
            {9 * Years, ext::make_shared<SimpleQuote>(0.014520)},
            {10 * Years, ext::make_shared<SimpleQuote>(0.015840)},
            {12 * Years, ext::make_shared<SimpleQuote>(0.018090)},
            {15 * Years, ext::make_shared<SimpleQuote>(0.020370)},
            {20 * Years, ext::make_shared<SimpleQuote>(0.021870)},
            {25 * Years, ext::make_shared<SimpleQuote>(0.022340)},
            {30 * Years, ext::make_shared<SimpleQuote>(0.022560)},
            {35 * Years, ext::make_shared<SimpleQuote>(0.022950)},
            {40 * Years, ext::make_shared<SimpleQuote>(0.023480)},
            {50 * Years, ext::make_shared<SimpleQuote>(0.024210)},
            {60 * Years, ext::make_shared<SimpleQuote>(0.024630)}
        };

        Frequency swFixedLegFrequency = Annual;
        BusinessDayConvention swFixedLegConvention = Unadjusted;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);

        for (auto q: swapQuotes) {
            auto tenor = q.first;
            auto quote = q.second;
            auto helper = ext::make_shared<SwapRateHelper>(
                Handle<Quote>(quote), tenor,
                calendar, swFixedLegFrequency,
                swFixedLegConvention, swFixedLegDayCounter,
                euribor6M,
                Handle<Quote>(), 0 * Days,
                discountingTermStructure); // the Eonia curve is used for discounting
            euribor6MInstruments.push_back(helper);
        }

        // If needed, it's possible to change the tolerance; the default is 1.0e-12.
        // The tolerance is passed in an explicit bootstrap object. Depending on
        // the bootstrap algorithm, it's possible to pass other parameters.
        double tolerance = 1.0e-15;
        ext::shared_ptr<YieldTermStructure> euribor6MTermStructure(
            new PiecewiseYieldCurve<Discount, Cubic>(
                settlementDate, euribor6MInstruments,
                termStructureDayCounter,
                PiecewiseYieldCurve<Discount, Cubic>::bootstrap_type(tolerance)));

        // This curve will be used for forward-rate forecasting

        RelinkableHandle<YieldTermStructure> forecastingTermStructure;
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

        auto s5yRate = swapQuotes[5 * Years];

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
