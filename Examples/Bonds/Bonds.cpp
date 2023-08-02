/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2008 Florent Grenier

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

/*  This example shows how to set up a term structure and then price
    some simple bonds. The last part is dedicated to peripherical
    computations such as "Yield to Price" or "Price to Yield"
 */

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

int main(int, char* []) {

    try {

        std::cout << std::endl;

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();

        Date settlementDate(18, September, 2008);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);

        Integer fixingDays = 3;
        Natural settlementDays = 3;

        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
        // nothing to do with Date::todaysDate
        Settings::instance().evaluationDate() = todaysDate;

        std::cout << "Today: " << todaysDate.weekday()
        << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
        << ", " << settlementDate << std::endl;


        // Building of the bonds discounting yield curve

        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // Common data

        // ZC rates for the short end
         Rate zc3mQuote=0.0096;
         Rate zc6mQuote=0.0145;
         Rate zc1yQuote=0.0194;

         ext::shared_ptr<Quote> zc3mRate(new SimpleQuote(zc3mQuote));
         ext::shared_ptr<Quote> zc6mRate(new SimpleQuote(zc6mQuote));
         ext::shared_ptr<Quote> zc1yRate(new SimpleQuote(zc1yQuote));

         DayCounter zcBondsDayCounter = Actual365Fixed();

         ext::shared_ptr<RateHelper> zc3m(new DepositRateHelper(
                 Handle<Quote>(zc3mRate),
                 3*Months, fixingDays,
                 calendar, ModifiedFollowing,
                 true, zcBondsDayCounter));
         ext::shared_ptr<RateHelper> zc6m(new DepositRateHelper(
                 Handle<Quote>(zc6mRate),
                 6*Months, fixingDays,
                 calendar, ModifiedFollowing,
                 true, zcBondsDayCounter));
         ext::shared_ptr<RateHelper> zc1y(new DepositRateHelper(
                 Handle<Quote>(zc1yRate),
                 1*Years, fixingDays,
                 calendar, ModifiedFollowing,
                 true, zcBondsDayCounter));

        // setup bonds
        Real redemption = 100.0;

        const Size numberOfBonds = 5;

        Date issueDates[] = {
                Date (15, March, 2005),
                Date (15, June, 2005),
                Date (30, June, 2006),
                Date (15, November, 2002),
                Date (15, May, 1987)
        };

        Date maturities[] = {
                Date (31, August, 2010),
                Date (31, August, 2011),
                Date (31, August, 2013),
                Date (15, August, 2018),
                Date (15, May, 2038)
        };

        Real couponRates[] = {
                0.02375,
                0.04625,
                0.03125,
                0.04000,
                0.04500
        };

        Real marketQuotes[] = {
                100.390625,
                106.21875,
                100.59375,
                101.6875,
                102.140625
        };

        std::vector< ext::shared_ptr<SimpleQuote> > quote;
        for (Real marketQuote : marketQuotes) {
            ext::shared_ptr<SimpleQuote> cp(new SimpleQuote(marketQuote));
            quote.push_back(cp);
        }

        RelinkableHandle<Quote> quoteHandle[numberOfBonds];
        for (Size i=0; i<numberOfBonds; i++) {
            quoteHandle[i].linkTo(quote[i]);
        }

        // Definition of the rate helpers
        std::vector<ext::shared_ptr<BondHelper> > bondsHelpers;

        for (Size i=0; i<numberOfBonds; i++) {

            Schedule schedule(issueDates[i], maturities[i], Period(Semiannual), UnitedStates(UnitedStates::GovernmentBond),
                    Unadjusted, Unadjusted, DateGeneration::Backward, false);

            ext::shared_ptr<FixedRateBondHelper> bondHelper(new FixedRateBondHelper(
                    quoteHandle[i],
                    settlementDays,
                    100.0,
                    schedule,
                    std::vector<Rate>(1,couponRates[i]),
                    ActualActual(ActualActual::Bond),
                    Unadjusted,
                    redemption,
                    issueDates[i]));

            // the above could also be done by creating a
            // FixedRateBond instance and writing:
            //
            // ext::shared_ptr<BondHelper> bondHelper(
            //         new BondHelper(quoteHandle[i], bond));
            //
            // This would also work for bonds that still don't have a
            // specialized helper, such as floating-rate bonds.


            bondsHelpers.push_back(bondHelper);
        }

        /*********************
         **  CURVE BUILDING **
         *********************/

         // Any DayCounter would be fine.
         // ActualActual::ISDA ensures that 30 years is 30.0
         DayCounter termStructureDayCounter =
             ActualActual(ActualActual::ISDA);

         // A depo-bond curve
         std::vector<ext::shared_ptr<RateHelper> > bondInstruments;

         // Adding the ZC bonds to the curve for the short end
         bondInstruments.push_back(zc3m);
         bondInstruments.push_back(zc6m);
         bondInstruments.push_back(zc1y);

         // Adding the Fixed rate bonds to the curve for the long end
         for (Size i=0; i<numberOfBonds; i++) {
             bondInstruments.push_back(bondsHelpers[i]);
         }

         ext::shared_ptr<YieldTermStructure> bondDiscountingTermStructure(
                 new PiecewiseYieldCurve<Discount,LogLinear>(
                         settlementDate, bondInstruments,
                         termStructureDayCounter));

         // Building of the Libor forecasting curve
         // deposits
         Rate d1wQuote=0.043375;
         Rate d1mQuote=0.031875;
         Rate d3mQuote=0.0320375;
         Rate d6mQuote=0.03385;
         Rate d9mQuote=0.0338125;
         Rate d1yQuote=0.0335125;
         // swaps
         Rate s2yQuote=0.0295;
         Rate s3yQuote=0.0323;
         Rate s5yQuote=0.0359;
         Rate s10yQuote=0.0412;
         Rate s15yQuote=0.0433;


         /********************
          ***    QUOTES    ***
          ********************/

         // SimpleQuote stores a value which can be manually changed;
         // other Quote subclasses could read the value from a database
         // or some kind of data feed.

         // deposits
         ext::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
         ext::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
         ext::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
         ext::shared_ptr<Quote> d6mRate(new SimpleQuote(d6mQuote));
         ext::shared_ptr<Quote> d9mRate(new SimpleQuote(d9mQuote));
         ext::shared_ptr<Quote> d1yRate(new SimpleQuote(d1yQuote));
         // swaps
         ext::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
         ext::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
         ext::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
         ext::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));
         ext::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));

         /*********************
          ***  RATE HELPERS ***
          *********************/

         // RateHelpers are built from the above quotes together with
         // other instrument dependant infos.  Quotes are passed in
         // relinkable handles which could be relinked to some other
         // data source later.

         // deposits
         DayCounter depositDayCounter = Actual360();

         ext::shared_ptr<RateHelper> d1w(new DepositRateHelper(
                 Handle<Quote>(d1wRate),
                 1*Weeks, fixingDays,
                 calendar, ModifiedFollowing,
                 true, depositDayCounter));
         ext::shared_ptr<RateHelper> d1m(new DepositRateHelper(
                 Handle<Quote>(d1mRate),
                 1*Months, fixingDays,
                 calendar, ModifiedFollowing,
                 true, depositDayCounter));
         ext::shared_ptr<RateHelper> d3m(new DepositRateHelper(
                 Handle<Quote>(d3mRate),
                 3*Months, fixingDays,
                 calendar, ModifiedFollowing,
                 true, depositDayCounter));
         ext::shared_ptr<RateHelper> d6m(new DepositRateHelper(
                 Handle<Quote>(d6mRate),
                 6*Months, fixingDays,
                 calendar, ModifiedFollowing,
                 true, depositDayCounter));
         ext::shared_ptr<RateHelper> d9m(new DepositRateHelper(
                 Handle<Quote>(d9mRate),
                 9*Months, fixingDays,
                 calendar, ModifiedFollowing,
                 true, depositDayCounter));
         ext::shared_ptr<RateHelper> d1y(new DepositRateHelper(
                 Handle<Quote>(d1yRate),
                 1*Years, fixingDays,
                 calendar, ModifiedFollowing,
                 true, depositDayCounter));

         // setup swaps
         Frequency swFixedLegFrequency = Annual;
         BusinessDayConvention swFixedLegConvention = Unadjusted;
         DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
         ext::shared_ptr<IborIndex> swFloatingLegIndex(new Euribor6M);

         const Period forwardStart(1*Days);

         ext::shared_ptr<RateHelper> s2y(new SwapRateHelper(
                 Handle<Quote>(s2yRate), 2*Years,
                 calendar, swFixedLegFrequency,
                 swFixedLegConvention, swFixedLegDayCounter,
                 swFloatingLegIndex, Handle<Quote>(),forwardStart));
         ext::shared_ptr<RateHelper> s3y(new SwapRateHelper(
                 Handle<Quote>(s3yRate), 3*Years,
                 calendar, swFixedLegFrequency,
                 swFixedLegConvention, swFixedLegDayCounter,
                 swFloatingLegIndex, Handle<Quote>(),forwardStart));
         ext::shared_ptr<RateHelper> s5y(new SwapRateHelper(
                 Handle<Quote>(s5yRate), 5*Years,
                 calendar, swFixedLegFrequency,
                 swFixedLegConvention, swFixedLegDayCounter,
                 swFloatingLegIndex, Handle<Quote>(),forwardStart));
         ext::shared_ptr<RateHelper> s10y(new SwapRateHelper(
                 Handle<Quote>(s10yRate), 10*Years,
                 calendar, swFixedLegFrequency,
                 swFixedLegConvention, swFixedLegDayCounter,
                 swFloatingLegIndex, Handle<Quote>(),forwardStart));
         ext::shared_ptr<RateHelper> s15y(new SwapRateHelper(
                 Handle<Quote>(s15yRate), 15*Years,
                 calendar, swFixedLegFrequency,
                 swFixedLegConvention, swFixedLegDayCounter,
                 swFloatingLegIndex, Handle<Quote>(),forwardStart));


         /*********************
          **  CURVE BUILDING **
          *********************/

         // Any DayCounter would be fine.
         // ActualActual::ISDA ensures that 30 years is 30.0

         // A depo-swap curve
         std::vector<ext::shared_ptr<RateHelper> > depoSwapInstruments;
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
         ext::shared_ptr<YieldTermStructure> depoSwapTermStructure(
                 new PiecewiseYieldCurve<Discount,LogLinear>(
                         settlementDate, depoSwapInstruments,
                         termStructureDayCounter));

         // Term structures that will be used for pricing:
         // the one used for discounting cash flows
         RelinkableHandle<YieldTermStructure> discountingTermStructure;
         // the one used for forward rate forecasting
         RelinkableHandle<YieldTermStructure> forecastingTermStructure;

         /*********************
          * BONDS TO BE PRICED *
          **********************/

         // Common data
         Real faceAmount = 100;

         // Pricing engine
         ext::shared_ptr<PricingEngine> bondEngine(
                 new DiscountingBondEngine(discountingTermStructure));

         // Zero coupon bond
         ZeroCouponBond zeroCouponBond(
                 settlementDays,
                 UnitedStates(UnitedStates::GovernmentBond),
                 faceAmount,
                 Date(15,August,2013),
                 Following,
                 Real(116.92),
                 Date(15,August,2003));

         zeroCouponBond.setPricingEngine(bondEngine);

         // Fixed 4.5% US Treasury Note
         Schedule fixedBondSchedule(Date(15, May, 2007),
                 Date(15,May,2017), Period(Semiannual),
                 UnitedStates(UnitedStates::GovernmentBond),
                 Unadjusted, Unadjusted, DateGeneration::Backward, false);

         FixedRateBond fixedRateBond(
                 settlementDays,
                 faceAmount,
                 fixedBondSchedule,
                 std::vector<Rate>(1, 0.045),
                 ActualActual(ActualActual::Bond),
                 ModifiedFollowing,
                 100.0, Date(15, May, 2007));

         fixedRateBond.setPricingEngine(bondEngine);

         // Floating rate bond (3M USD Libor + 0.1%)
         // Should and will be priced on another curve later...

         RelinkableHandle<YieldTermStructure> liborTermStructure;
         const ext::shared_ptr<IborIndex> libor3m(
                 new USDLibor(Period(3,Months),liborTermStructure));
         libor3m->addFixing(Date(17, July, 2008),0.0278625);

         Schedule floatingBondSchedule(Date(21, October, 2005),
                 Date(21, October, 2010), Period(Quarterly),
                 UnitedStates(UnitedStates::NYSE),
                 Unadjusted, Unadjusted, DateGeneration::Backward, true);

         FloatingRateBond floatingRateBond(
                 settlementDays,
                 faceAmount,
                 floatingBondSchedule,
                 libor3m,
                 Actual360(),
                 ModifiedFollowing,
                 Natural(2),
                 // Gearings
                 std::vector<Real>(1, 1.0),
                 // Spreads
                 std::vector<Rate>(1, 0.001),
                 // Caps
                 std::vector<Rate>(),
                 // Floors
                 std::vector<Rate>(),
                 // Fixing in arrears
                 true,
                 Real(100.0),
                 Date(21, October, 2005));

         floatingRateBond.setPricingEngine(bondEngine);

         // Coupon pricers
         auto pricer = ext::make_shared<BlackIborCouponPricer>();

         // optionLet volatilities
         Volatility volatility = 0.0;
         Handle<OptionletVolatilityStructure> vol;
         vol = Handle<OptionletVolatilityStructure>(
                 ext::shared_ptr<OptionletVolatilityStructure>(new
                         ConstantOptionletVolatility(
                                 settlementDays,
                                 calendar,
                                 ModifiedFollowing,
                                 volatility,
                                 Actual365Fixed())));

         pricer->setCapletVolatility(vol);
         setCouponPricer(floatingRateBond.cashflows(),pricer);

         // Yield curve bootstrapping
         forecastingTermStructure.linkTo(depoSwapTermStructure);
         discountingTermStructure.linkTo(bondDiscountingTermStructure);

         // We are using the depo & swap curve to estimate the future Libor rates
         liborTermStructure.linkTo(depoSwapTermStructure);

         /***************
          * BOND PRICING *
          ****************/

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

         std::cout << std::setw(widths[0]) << "Net present value"
         << std::setw(widths[1]) << zeroCouponBond.NPV()
         << std::setw(widths[2]) << fixedRateBond.NPV()
         << std::setw(widths[3]) << floatingRateBond.NPV()
         << std::endl;

         std::cout << std::setw(widths[0]) << "Clean price"
         << std::setw(widths[1]) << zeroCouponBond.cleanPrice()
         << std::setw(widths[2]) << fixedRateBond.cleanPrice()
         << std::setw(widths[3]) << floatingRateBond.cleanPrice()
         << std::endl;

         std::cout << std::setw(widths[0]) << "Dirty price"
         << std::setw(widths[1]) << zeroCouponBond.dirtyPrice()
         << std::setw(widths[2]) << fixedRateBond.dirtyPrice()
         << std::setw(widths[3]) << floatingRateBond.dirtyPrice()
         << std::endl;

         std::cout << std::setw(widths[0]) << "Accrued coupon"
         << std::setw(widths[1]) << zeroCouponBond.accruedAmount()
         << std::setw(widths[2]) << fixedRateBond.accruedAmount()
         << std::setw(widths[3]) << floatingRateBond.accruedAmount()
         << std::endl;

         std::cout << std::setw(widths[0]) << "Previous coupon"
         << std::setw(widths[1]) << "N/A" // zeroCouponBond
         << std::setw(widths[2]) << io::rate(fixedRateBond.previousCouponRate())
         << std::setw(widths[3]) << io::rate(floatingRateBond.previousCouponRate())
         << std::endl;

         std::cout << std::setw(widths[0]) << "Next coupon"
         << std::setw(widths[1]) << "N/A" // zeroCouponBond
         << std::setw(widths[2]) << io::rate(fixedRateBond.nextCouponRate())
         << std::setw(widths[3]) << io::rate(floatingRateBond.nextCouponRate())
         << std::endl;

         std::cout << std::setw(widths[0]) << "Yield"
         << std::setw(widths[1])
         << io::rate(zeroCouponBond.yield(Actual360(),Compounded,Annual))
         << std::setw(widths[2])
         << io::rate(fixedRateBond.yield(Actual360(),Compounded,Annual))
         << std::setw(widths[3])
         << io::rate(floatingRateBond.yield(Actual360(),Compounded,Annual))
         << std::endl;

         std::cout << std::endl;

         // Other computations
         std::cout << "Sample indirect computations (for the floating rate bond): " << std::endl;
         std::cout << rule << std::endl;

         std::cout << "Yield to Clean Price: "
         << floatingRateBond.cleanPrice(floatingRateBond.yield(Actual360(),Compounded,Annual),Actual360(),Compounded,Annual,settlementDate) << std::endl;

         std::cout << "Clean Price to Yield: "
         << io::rate(floatingRateBond.yield(floatingRateBond.cleanPrice(),Actual360(),Compounded,Annual,settlementDate)) << std::endl;

         /* "Yield to Price"
            "Price to Yield" */

         return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

