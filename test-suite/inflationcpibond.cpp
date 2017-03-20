/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon
 Copyright (C) 2012 StatPro Italia srl
 
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

#include "inflationcpibond.hpp"
#include "utilities.hpp"
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/indexes/inflation/zacpi.hpp>
#include <ql/time/calendars/southafrica.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/indexes/ibor/gbplibor.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/cpicouponpricer.hpp>
#include <ql/instruments/cpiswap.hpp>
#include <ql/instruments/bonds/cpibond.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#include <iostream>

namespace {

    struct Datum {
        Date date;
        Rate rate;
    };

    typedef BootstrapHelper<ZeroInflationTermStructure> Helper;

    std::vector<boost::shared_ptr<Helper> > makeHelpers(
        Datum iiData[], Size N,
        const boost::shared_ptr<ZeroInflationIndex>& ii,
        const Period& observationLag,
        const Calendar& calendar,
        const BusinessDayConvention& bdc,
        const DayCounter& dc) {

        std::vector<boost::shared_ptr<Helper> > instruments;
        for (Size i=0; i<N; i++) {
            Date maturity = iiData[i].date;
            Handle<Quote> quote(boost::shared_ptr<Quote>(
                                new SimpleQuote(iiData[i].rate/100.0)));
            boost::shared_ptr<Helper> h(
                      new ZeroCouponInflationSwapHelper(quote, observationLag,
                                                        maturity, calendar,
                                                        bdc, dc, ii));
            instruments.push_back(h);
        }
        return instruments;
    }


    struct UKCommonVars {

        Calendar calendar;
        BusinessDayConvention convention;
        Date evaluationDate;
        Period observationLag;
        DayCounter dayCounter;

        boost::shared_ptr<UKRPI> ii;

        RelinkableHandle<YieldTermStructure> yTS;
        RelinkableHandle<ZeroInflationTermStructure> cpiTS;

        // cleanup

        SavedSettings backup;
        IndexHistoryCleaner cleaner;

        // setup
        UKCommonVars() {
            // usual setup
            calendar = UnitedKingdom();
            convention = ModifiedFollowing;
            Date today(25, November, 2009);
            evaluationDate = calendar.adjust(today);
            Settings::instance().evaluationDate() = evaluationDate;
            dayCounter = ActualActual();

            Date from(20, July, 2007);
            Date to(20, November, 2009);
            Schedule rpiSchedule =
                MakeSchedule().from(from).to(to)
                .withTenor(1*Months)
                .withCalendar(UnitedKingdom())
                .withConvention(ModifiedFollowing);

            bool interp = false;
            ii = boost::shared_ptr<UKRPI>(new UKRPI(interp, cpiTS));

            Real fixData[] = {
                206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
                209.8, 211.4, 212.1, 214.0, 215.1, 216.8,
                216.5, 217.2, 218.4, 217.7, 216,
                212.9, 210.1, 211.4, 211.3, 211.5,
                212.8, 213.4, 213.4, 213.4, 214.4
            };
            for (Size i=0; i<LENGTH(fixData); ++i) {
                ii->addFixing(rpiSchedule[i], fixData[i]);
            }

            yTS.linkTo(boost::shared_ptr<YieldTermStructure>(
                          new FlatForward(evaluationDate, 0.05, dayCounter)));

            // now build the zero inflation curve
            observationLag = Period(2,Months);

            Datum zciisData[] = {
                { Date(25, November, 2010), 3.0495 },
                { Date(25, November, 2011), 2.93 },
                { Date(26, November, 2012), 2.9795 },
                { Date(25, November, 2013), 3.029 },
                { Date(25, November, 2014), 3.1425 },
                { Date(25, November, 2015), 3.211 },
                { Date(25, November, 2016), 3.2675 },
                { Date(25, November, 2017), 3.3625 },
                { Date(25, November, 2018), 3.405 },
                { Date(25, November, 2019), 3.48 },
                { Date(25, November, 2021), 3.576 },
                { Date(25, November, 2024), 3.649 },
                { Date(26, November, 2029), 3.751 },
                { Date(27, November, 2034), 3.77225 },
                { Date(25, November, 2039), 3.77 },
                { Date(25, November, 2049), 3.734 },
                { Date(25, November, 2059), 3.714 },
            };

            std::vector<boost::shared_ptr<Helper> > helpers =
                makeHelpers(zciisData, LENGTH(zciisData), ii,
                            observationLag, calendar, convention, dayCounter);

            Rate baseZeroRate = zciisData[0].rate/100.0;
            cpiTS.linkTo(boost::shared_ptr<ZeroInflationTermStructure>(
                  new PiecewiseZeroInflationCurve<Linear>(
                         evaluationDate, calendar, dayCounter, observationLag,
                         ii->frequency(),ii->interpolated(), baseZeroRate,
                         Handle<YieldTermStructure>(yTS), helpers)));
        }

        // teardown
        ~UKCommonVars() {
            // break circular references and allow curves to be destroyed
            cpiTS.linkTo(boost::shared_ptr<ZeroInflationTermStructure>());
        }
    };

    struct ZACommonVars {

        Calendar calendar;
        BusinessDayConvention convention;
        Natural settlementDays;
        Period observationLag;
        DayCounter dayCounter;

        boost::shared_ptr<ZeroInflationIndex> ii;

        RelinkableHandle<YieldTermStructure> yTS;
        RelinkableHandle<ZeroInflationTermStructure> cpiTS;

        // cleanup

        SavedSettings backup;
        IndexHistoryCleaner cleaner;

        // setup
        ZACommonVars(Date evaluationDate, Date settlementDate) {
            calendar = NullCalendar();
            convention = ModifiedFollowing;
            settlementDays = calendar.businessDaysBetween(evaluationDate, settlementDate);
            dayCounter = ActualActual();
            observationLag = Period(4, Months);

            bool interp = false;
            Period indexAvailability(1, Months);
            ii = boost::shared_ptr<ZeroInflationIndex>(
                new ZeroInflationIndex("CPI", ZARegion(), false, true, Monthly,
                    indexAvailability, ZARCurrency(), cpiTS, true));

            Datum cpiFixings[] = {
                { Date(31, Jan, 2015), 110.8 },
                { Date(28, Feb, 2015), 111.5 },
                { Date(31, Mar, 2015), 113.1 },
                { Date(30, Apr, 2015), 114.1 },
                { Date(31, May, 2015), 114.4 },
                { Date(30, Jun, 2015), 114.9 },
                { Date(31, Jul, 2015), 116.1 },
                { Date(31, Aug, 2015), 116.1 },
                { Date(30, Sep, 2015), 116.1 },
                { Date(31, Oct, 2015), 116.4 },
                { Date(30, Nov, 2015), 116.5 },
                { Date(31, Dec, 2015), 116.8 },
                { Date(31, Jan, 2016), 117.7 },
                { Date(29, Feb, 2016), 119.3 },
                { Date(31, Mar, 2016), 120.2 },
                { Date(30, Apr, 2016), 121.2 },
                { Date(31, May, 2016), 121.4 },
                { Date(30, Jun, 2016), 122.1 },
                { Date(31, Jul, 2016), 123.1 },
                { Date(31, Aug, 2016), 123 },
                { Date(30, Sep, 2016), 123.2 },
                { Date(31, Oct, 2016), 123.8 },
                { Date(30, Nov, 2016), 124.2 },
                { Date(31, Dec, 2016), 124.7 },
                { Date(31, Jan, 2017), 124.7 }
            };

            for (Size i = 0; i < LENGTH(cpiFixings); ++i) {
                ii->addFixing(cpiFixings[i].date, cpiFixings[i].rate);
            }

            yTS.linkTo(boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, 0.50, dayCounter)));

            std::vector<Date> dates;
            dates.push_back(calendar.advance(settlementDate, -observationLag));
            dates.push_back(Date(1, May, 2100));

            std::vector<Rate> rates;
            rates.push_back(0);
            rates.push_back(0);

            boost::shared_ptr< InterpolatedZeroInflationCurve<Linear> > izic = boost::shared_ptr< InterpolatedZeroInflationCurve<Linear> >(new InterpolatedZeroInflationCurve<Linear>(
                settlementDate, calendar, Actual365Fixed(), observationLag,
                ii->frequency(), true, Handle<YieldTermStructure>(yTS), dates, rates));

            cpiTS.linkTo(izic);
        }

        // teardown
        ~ZACommonVars() {
            // break circular references and allow curves to be destroyed
            cpiTS.linkTo(boost::shared_ptr<ZeroInflationTermStructure>());
        }
    };
}


void InflationCPIBondTest::testCleanPrice() {
    BOOST_TEST_MESSAGE("Testing UK CPI bond reference period on 2009-11-25...");

    IndexManager::instance().clearHistories();
  
    UKCommonVars common;

    Real notional = 1000000.0;
    std::vector<Rate> fixedRates(1, 0.1);
    DayCounter fixedDayCount = Actual365Fixed();
    BusinessDayConvention fixedPaymentConvention = ModifiedFollowing;
    Calendar fixedPaymentCalendar = UnitedKingdom();
    boost::shared_ptr<ZeroInflationIndex> fixedIndex = common.ii;
    Period contractObservationLag = Period(3,Months);
    CPI::InterpolationType observationInterpolation = CPI::Flat;
    Natural settlementDays = 3;
    bool growthOnly = true;

    Real baseCPI = 206.1;
    // set the schedules
    Date startDate(2, October, 2007);
    Date endDate(2, October, 2052);
    Schedule fixedSchedule =
        MakeSchedule().from(startDate).to(endDate)
                      .withTenor(Period(6,Months))
                      .withCalendar(UnitedKingdom())
                      .withConvention(Unadjusted)
                      .backwards();

    CPIBond bond(settlementDays, notional, growthOnly,
                 baseCPI, contractObservationLag, fixedIndex,
                 observationInterpolation, fixedSchedule,
                 fixedRates, fixedDayCount, fixedPaymentConvention);

    boost::shared_ptr<DiscountingBondEngine> engine(
                                 new DiscountingBondEngine(common.yTS));
    bond.setPricingEngine(engine);

    Real storedPrice = 383.01816406;
    Real calculated = bond.cleanPrice();
    Real tolerance = 1.0e-8;
    if (std::fabs(calculated-storedPrice) > tolerance) {
        BOOST_FAIL("failed to reproduce expected CPI-bond clean price"
                   << QL_FIXED << std::setprecision(12)
                   << "\n  expected:   " << storedPrice
                   << "\n  calculated: " << calculated);
    }
}

void InflationCPIBondTest::testZABondsReferencePeriod() {
    BOOST_TEST_MESSAGE("Testing CPI bond reference period on 2015-07-31...");

    IndexManager::instance().clearHistories();

    struct test_case {
        Date issueDate;
        Date maturityDate;
        Real couponRate;
        Real ytm;
        Real baseCPI;
        bool couponsAtEndOfMonth;
        Rate dirtyPrice;
    };

    Real notional = 100.0;
    DayCounter paymentDayCounter = ActualActual(ActualActual::ISMA);
    DayCounter accruedInterestDayCounter = Actual365Fixed();
    BusinessDayConvention paymentConvention = ModifiedFollowing;
    CPI::InterpolationType observationInterpolation = CPI::Linear;
    bool growthOnly = false;

    // Tests for trade date 2015-07-31
    Date evaluationDate(31, Jul, 2015);
    Date settlementDate(5, Aug, 2015);
    ZACommonVars common(evaluationDate, settlementDate);
    Settings::instance().evaluationDate() = evaluationDate;

    test_case cases[] = {
        { Date( 9, Jun, 2010), Date(31, Jan, 2017), 0.0250, 0.00825, 87.16653512,  true, 134.22128 }, // R211 bond
        { Date(17, Jun, 2010), Date(31, Jan, 2022), 0.0275, 0.01445, 87.35595896,  true, 141.22928 }, // R212 bond
        { Date(30, May, 2001), Date( 7, Dec, 2023), 0.0550, 0.01525, 51.34206788, false, 293.22573 }, // R197 bond
        { Date( 4, Jul, 2012), Date(31, Jan, 2025), 0.0200, 0.01515, 96.80219968,  true, 122.97651 }, // I2025 bond
        { Date(27, Sep, 2007), Date(31, Mar, 2028), 0.0260, 0.01625, 70.46172060,  true, 181.44388 }, // R210 bond
        { Date(20, Aug, 2003), Date( 7, Dec, 2033), 0.0345, 0.01780, 60.63344751, false, 238.29050 }, // R202 bond
        { Date( 4, Jul, 2012), Date(31, Jan, 2038), 0.0225, 0.01810, 96.80219732,  true, 127.49386 }, // I2038 bond
        { Date(17, Jul, 2013), Date(31, Mar, 2046), 0.0250, 0.01950, 102.7064515,  true, 126.14694 }, // I2046 bond
        { Date(11, Jul, 2012), Date(31, Dec, 2050), 0.0250, 0.01925, 96.89131176,  true, 135.41703 }  // I2050 bond
    };

    for (Size i = 0; i < LENGTH(cases); ++i) {
        Schedule schedule(
            cases[i].issueDate,
            cases[i].maturityDate,
            Period(6, Months),
            common.calendar,
            Unadjusted,
            Unadjusted,
            DateGeneration::Backward,
            cases[i].couponsAtEndOfMonth);

        std::vector<Rate> couponRate(1, cases[i].couponRate);

        CPIBond bond(common.settlementDays, notional, growthOnly,
            cases[i].baseCPI, common.observationLag, common.ii,
            observationInterpolation, schedule,
            couponRate, paymentDayCounter, paymentConvention,
            cases[i].issueDate,
            common.calendar,
            //Ex-coupon parameters
            Period(10, Days),
            NullCalendar(),
            Unadjusted,
            false);

        Real calculated = BondFunctions::dirtyPrice(bond,
            InterestRate(cases[i].ytm, paymentDayCounter, Compounded, Semiannual),
            settlementDate);

        Real tolerance = 2e-5;
        if (std::fabs(calculated - cases[i].dirtyPrice) > tolerance) {
            BOOST_FAIL("failed to reproduce expected ZA CPI-bond dirty price on 2015-07-31"
                << QL_FIXED << std::setprecision(12)
                << "\n  expected: " << cases[i].dirtyPrice
                << "\n  actual  : " << calculated);
        }
    }
}

void InflationCPIBondTest::testZABondsReferencePeriod2() {
    BOOST_TEST_MESSAGE("Testing CPI bond reference period on 2017-02-14...");

    IndexManager::instance().clearHistories();

    struct test_case {
        Date issueDate;
        Date maturityDate;
        Real couponRate;
        Real ytm;
        Real baseCPI;
        bool couponsAtEndOfMonth;
        Rate dirtyPrice;
    };

    Real notional = 100.0;
    DayCounter paymentDayCounter = ActualActual(ActualActual::ISMA);
    DayCounter accruedInterestDayCounter = Actual365Fixed();
    BusinessDayConvention paymentConvention = ModifiedFollowing;
    CPI::InterpolationType observationInterpolation = CPI::Linear;
    bool growthOnly = false;

    // Tests for trade date 2017-02-14
    Date evaluationDate(14, Feb, 2017);
    Date settlementDate(17, Feb, 2017);
    ZACommonVars common(evaluationDate, settlementDate);
    Settings::instance().evaluationDate() = evaluationDate;

    test_case cases[] = {
        { Date(17, Jun, 2010), Date(31, Jan, 2022), 0.0275, 0.02020,  87.35595896,  true, 147.02504}, // R212 bond
        { Date(30, May, 2001), Date( 7, Dec, 2023), 0.0550, 0.02040,  51.34207043, false, 297.02942}, // R197 bond
        { Date( 4, Jul, 2012), Date(31, Jan, 2025), 0.0200, 0.02080,  96.80219968,  true, 127.49778}, // I2025 bond
        { Date(27, Sep, 2007), Date(31, Mar, 2028), 0.0260, 0.02020,  70.46172060,  true, 187.89960}, // R210 bond
        { Date(20, Aug, 2003), Date( 7, Dec, 2033), 0.0345, 0.02070,  60.63344961, false, 245.83447}, // R202 bond
        { Date( 4, Jul, 2012), Date(31, Jan, 2038), 0.0225, 0.02035,  96.80219976,  true, 132.94059}, // I2038 bond
        { Date(17, Jul, 2013), Date(31, Mar, 2046), 0.0250, 0.02080, 102.70645160,  true, 132.95497}, // I2046 bond
        { Date(11, Jul, 2012), Date(31, Dec, 2050), 0.0250, 0.02040,  96.89131042,  true, 142.77971}  // I2050 bond
    };

    for (Size i = 0; i < LENGTH(cases); ++i) {
        Schedule schedule(
            cases[i].issueDate,
            cases[i].maturityDate,
            Period(6, Months),
            common.calendar,
            Unadjusted,
            Unadjusted,
            DateGeneration::Backward,
            cases[i].couponsAtEndOfMonth);

        std::vector<Rate> couponRate(1, cases[i].couponRate);

        CPIBond bond(common.settlementDays, notional, growthOnly,
            cases[i].baseCPI, common.observationLag, common.ii,
            observationInterpolation, schedule,
            couponRate, paymentDayCounter, paymentConvention,
            cases[i].issueDate,
            schedule.calendar(),
            //Ex-coupon parameters
            Period(10, Days),
            NullCalendar(),
            Unadjusted,
            false);

        Real calculated = BondFunctions::dirtyPrice(bond,
            InterestRate(cases[i].ytm, paymentDayCounter, Compounded, Semiannual),
            settlementDate);

        Real tolerance = 2e-5;
        if (std::fabs(calculated - cases[i].dirtyPrice) > tolerance) {
            BOOST_FAIL("failed to reproduce expected ZA CPI-bond dirty price on 2017-02-14"
                << QL_FIXED << std::setprecision(12)
                << "\n  expected: " << cases[i].dirtyPrice
                << "\n  actual  : " << calculated);
        }
    }
}


test_suite* InflationCPIBondTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CPI bond tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationCPIBondTest::testCleanPrice));
    suite->add(QUANTLIB_TEST_CASE(&InflationCPIBondTest::testZABondsReferencePeriod));
    suite->add(QUANTLIB_TEST_CASE(&InflationCPIBondTest::testZABondsReferencePeriod2));

    return suite;
}

