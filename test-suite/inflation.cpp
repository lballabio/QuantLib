/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2008 Piero Del Boca

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
#include <ql/indexes/inflation/ukhicp.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/schedule.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/instruments/yearonyearinflationswap.hpp>
#include <functional>

using boost::unit_test_framework::test_suite;
using namespace QuantLib;

using std::fabs;
using std::pow;
using std::vector;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(InflationTests)

#undef REPORT_FAILURE
#define REPORT_FAILURE(d, res, periodName) \
    BOOST_ERROR("wrong " << periodName << " inflation period for Date (1 " \
        << d << "), Start Date ( " \
        << res.first << "), End Date (" \
        << res.second << ")"); \

struct Datum {
    Date date;
    Rate rate;
};

ext::shared_ptr<YieldTermStructure> nominalTermStructure() {
    Date evaluationDate(13, August, 2007);
    return ext::shared_ptr<YieldTermStructure>(
            new FlatForward(evaluationDate, 0.05, Actual360()));
}

template <class T>
std::vector<ext::shared_ptr<BootstrapHelper<T> > > makeHelpers(
        const std::vector<Datum>& iiData,
        std::function<ext::shared_ptr<BootstrapHelper<T> >(const Handle<Quote>&, const Date&)>
            makeHelper) {

    std::vector<ext::shared_ptr<BootstrapHelper<T> > > instruments;
    for (Datum datum : iiData) {
        Date maturity = datum.date;
        Handle<Quote> quote(ext::shared_ptr<Quote>(new SimpleQuote(datum.rate / 100.0)));
        auto anInstrument = makeHelper(quote, maturity);
        instruments.push_back(anInstrument);
    }

    return instruments;
}

void checkSeasonality(const Handle<ZeroInflationTermStructure>& hz,
                      const ext::shared_ptr<ZeroInflationIndex>& ii) {

    QL_REQUIRE(!hz->hasSeasonality(), "We require that the initially passed in term structure "
               << "does not have seasonality");

    // Tolerance that we will use below when comparing projected index fixings
    Rate tolerance = 1e-12;

    Date trueBaseDate = inflationPeriod(hz->baseDate(), ii->frequency()).second;
    Date seasonalityBaseDate(31, January, trueBaseDate.year());

    // Create two different seasonality objects

    // 1) Monthly seasonality with all elements equal to 1 <=> no seasonality
    vector<Rate> seasonalityFactors(12, 1.0);
    ext::shared_ptr<MultiplicativePriceSeasonality> unitSeasonality =
        ext::make_shared<MultiplicativePriceSeasonality>(seasonalityBaseDate, Monthly, seasonalityFactors);

    // 2) Seasonality with factors != 1.0
    seasonalityFactors[0] = 1.003245;
    seasonalityFactors[1] = 1.000000;
    seasonalityFactors[2] = 0.999715;
    seasonalityFactors[3] = 1.000495;
    seasonalityFactors[4] = 1.000929;
    seasonalityFactors[5] = 0.998687;
    seasonalityFactors[6] = 0.995949;
    seasonalityFactors[7] = 0.994682;
    seasonalityFactors[8] = 0.995949;
    seasonalityFactors[9] = 1.000519;
    seasonalityFactors[10] = 1.003705;
    seasonalityFactors[11] = 1.004186;

    ext::shared_ptr<MultiplicativePriceSeasonality> nonUnitSeasonality =
        ext::make_shared<MultiplicativePriceSeasonality>(seasonalityBaseDate, Monthly, seasonalityFactors);

    // Create dates on which we will check fixings
    vector<Date> fixingDates(12);
    Date anchorDate(14, January, 2013);
    for (Size i = 0; i < fixingDates.size(); ++i) {
        fixingDates[i] = anchorDate + i * Months;
    }

    // Projected inflation index fixings when there is no seasonality
    vector<Rate> noSeasonalityFixings(12, 1.0);
    for (Size i = 0; i < fixingDates.size(); ++i) {
        noSeasonalityFixings[i] = ii->fixing(fixingDates[i], true);
    }

    // Set seasonality of all 1's and get the projected index fixings
    hz->setSeasonality(unitSeasonality);
    vector<Rate> unitSeasonalityFixings(12, 1.0);
    for (Size i = 0; i < fixingDates.size(); ++i) {
        unitSeasonalityFixings[i] = ii->fixing(fixingDates[i], true);
    }

    // Check that the unit seasonality fixings agree with the no seasonality fixings
    for (Size i = 0; i < fixingDates.size(); i++) {
        if (fabs(noSeasonalityFixings[i] - unitSeasonalityFixings[i]) > tolerance) {
            BOOST_ERROR("Seasonality doesn't work correctly when seasonality factors are set = 1"
                        << "No seasonality fixing is: " << noSeasonalityFixings[i]
                        << " but unit seasonality fixing is: " << unitSeasonalityFixings[i]
                        << " for fixing date " << io::iso_date(fixingDates[i]));
        }
    }

    // Testing seasonality correction when seasonality factors are different from 1
    // We expect to see that I_{SA}(t) = I_{NSA}(t) * S(t) / S(t_b)
    Month baseCpiMonth = hz->baseDate().month();
    Size baseCpiIndex = static_cast<Size>(baseCpiMonth) - 1;
    Rate baseSeasonality = seasonalityFactors[baseCpiIndex];

    // These are the expected fixings
    vector<Rate> expectedSeasonalityFixings(12, 1.0);
    for (Size i = 0; i < expectedSeasonalityFixings.size(); ++i) {
        expectedSeasonalityFixings[i] =
            ii->fixing(fixingDates[i], true) * seasonalityFactors[i] / baseSeasonality;
    }

    // Set the seasonality and calculate the actual seasonally adjusted fixings
    hz->setSeasonality(nonUnitSeasonality);
    vector<Rate> nonUnitSeasonalityFixings(12, 1.0);
    for (Size i = 0; i < fixingDates.size(); ++i) {
        nonUnitSeasonalityFixings[i] = ii->fixing(fixingDates[i], true);
    }

    // Check that the calculated fixings agree with the expected fixings
    for (Size i = 0; i < fixingDates.size(); i++) {
        if (fabs(expectedSeasonalityFixings[i] - nonUnitSeasonalityFixings[i]) > tolerance) {
            BOOST_ERROR("Seasonality doesn't work correctly for non-unit seasonality factors."
                        << " Expected fixing is: " << expectedSeasonalityFixings[i]
                        << " but calculated fixing is: " << nonUnitSeasonalityFixings[i]
                        << " for fixing date " << io::iso_date(fixingDates[i]));
        }
    }

    // Testing that unsetting seasonality works also
    hz->setSeasonality({});
    vector<Rate> unsetSeasonalityFixings(12, 1.0);
    for (Size i = 0; i < fixingDates.size(); ++i) {
        unsetSeasonalityFixings[i] = ii->fixing(fixingDates[i], true);
    }

    // Check that seasonality has been unset by comparing with the no seasonality fixings
    for (Size i = 0; i < fixingDates.size(); i++) {
        if (fabs(noSeasonalityFixings[i] - unsetSeasonalityFixings[i]) > tolerance) {
            BOOST_ERROR("Unsetting seasonality doesn't work correctly."
                        << " No seasonality fixing is: " << noSeasonalityFixings[i]
                        << " but after unsetting seasonality fixing is: " << unitSeasonalityFixings[i]
                        << " for fixing date " << io::iso_date(fixingDates[i]));
        }
    }
}


//===========================================================================================
// zero inflation tests, index, termstructure, and swaps
//===========================================================================================

BOOST_AUTO_TEST_CASE(testZeroIndex) {
    BOOST_TEST_MESSAGE("Testing zero inflation indices...");

    EUHICP euhicp;
    if (euhicp.name() != "EU HICP"
        || euhicp.frequency() != Monthly
        || euhicp.revised()
        || euhicp.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong EU HICP data ("
                    << euhicp.name() << ", "
                    << euhicp.frequency() << ", "
                    << euhicp.revised() << ", "
                    << euhicp.availabilityLag() << ")");
    }

    UKRPI ukrpi;
    if (ukrpi.name() != "UK RPI"
        || ukrpi.frequency() != Monthly
        || ukrpi.revised()
        || ukrpi.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong UK RPI data ("
                    << ukrpi.name() << ", "
                    << ukrpi.frequency() << ", "
                    << ukrpi.revised() << ", "
                    << ukrpi.availabilityLag() << ")");
    }

    UKHICP ukhicp;
    if (ukhicp.name() != "UK HICP"
        || ukhicp.frequency() != Monthly
        || ukhicp.revised() 
        || ukhicp.availabilityLag() != 1 * Months) {
        BOOST_ERROR("wrong UK HICP data ("
                    << ukhicp.name() << ", "
                    << ukhicp.frequency() << ", "
                    << ukhicp.revised() << ", "
                    << ", " << ukhicp.availabilityLag() << ")");
    }

    // Retrieval test.
    //----------------
    // make sure of the evaluation date
    Date evaluationDate(13, August, 2007);
    evaluationDate = UnitedKingdom().adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
    .withTenor(1*Months)
    .withCalendar(UnitedKingdom())
    .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3, 206.1 };

    auto iir = ext::make_shared<UKRPI>();
    for (Size i=0; i<LENGTH(fixData); i++) {
        iir->addFixing(rpiSchedule[i], fixData[i]);
    }

    Date todayMinusLag = evaluationDate - iir->availabilityLag();
    std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, iir->frequency());
    todayMinusLag = lim.first;

    Real eps = 1.0e-8;

    // -1 because last value not yet available,
    // (no TS so can't forecast).
    for (Size i=0; i<rpiSchedule.size()-1;i++) {
        std::pair<Date,Date> lim = inflationPeriod(rpiSchedule[i],
                                                   iir->frequency());
        for (Date d=lim.first; d<=lim.second; d++) {
            if (d < inflationPeriod(todayMinusLag,iir->frequency()).first) {
                if (std::fabs(iir->fixing(d) - fixData[i]) > eps)
                    BOOST_ERROR("Fixings not constant within a period: "
                                << iir->fixing(d)
                                << ", should be " << fixData[i]);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testZeroTermStructure) {
    BOOST_TEST_MESSAGE("Testing zero inflation term structure...");

    // try the Zero UK
    Calendar calendar = UnitedKingdom();
    BusinessDayConvention bdc = ModifiedFollowing;
    Date evaluationDate(13, August, 2007);
    evaluationDate = calendar.adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    Date baseDate(1, July, 2007);

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
        .withTenor(1*Months)
        .withCalendar(NullCalendar())
        .withConvention(ModifiedFollowing)
        .forwards();

    Real fixData[] = {
        189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3};

    RelinkableHandle<ZeroInflationTermStructure> hz;
    auto ii = ext::make_shared<UKRPI>(hz);
    for (Size i=0; i<LENGTH(fixData); i++) {
        ii->addFixing(rpiSchedule[i], fixData[i]);
    }

    Handle<YieldTermStructure> nominalTS(nominalTermStructure());

    // now build the zero inflation curve
    std::vector<Datum> zcData = {
        { Date(13, August, 2008), 2.93 },
        { Date(13, August, 2009), 2.95 },
        { Date(13, August, 2010), 2.965 },
        { Date(15, August, 2011), 2.98 },
        { Date(13, August, 2012), 3.0 },
        { Date(13, August, 2014), 3.06 },
        { Date(13, August, 2017), 3.175 },
        { Date(13, August, 2019), 3.243 },
        { Date(15, August, 2022), 3.293 },
        { Date(14, August, 2027), 3.338 },
        { Date(13, August, 2032), 3.348 },
        { Date(15, August, 2037), 3.348 },
        { Date(13, August, 2047), 3.308 },
        { Date(13, August, 2057), 3.228 }
    };

    Period observationLag = Period(3, Months);
    DayCounter dc = Thirty360(Thirty360::BondBasis);
    Frequency frequency = Monthly;

    auto makeHelper = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<ZeroCouponInflationSwapHelper>(
            quote, observationLag, maturity, calendar, bdc, dc, ii, CPI::AsIndex, nominalTS);
    };
    auto helpers = makeHelpers<ZeroInflationTermStructure>(zcData, makeHelper);
    Rate baseZeroRate = zcData[0].rate/100.0;

    ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITS =
        ext::make_shared<PiecewiseZeroInflationCurve<Linear>>(
            evaluationDate, baseDate, frequency, dc, helpers, baseZeroRate);
    hz.linkTo(pZITS);

    //===========================================================================================
    // first check that the quoted swaps are repriced correctly

    const Real eps = 1.0e-7;
    auto engine = ext::make_shared<DiscountingSwapEngine>(nominalTS);

    for (const auto& datum: zcData) {
        ZeroCouponInflationSwap nzcis(Swap::Payer,
                                      1000000.0,
                                      evaluationDate,
                                      datum.date,
                                      calendar, bdc, dc,
                                      datum.rate/100.0,
                                      ii, observationLag,
                                      CPI::AsIndex);
        nzcis.setPricingEngine(engine);

        BOOST_CHECK_MESSAGE(std::fabs(nzcis.NPV()) < eps,
                            "zero-coupon inflation swap does not reprice to zero"
                            << "\n    NPV:      " << nzcis.NPV()
                            << "\n    maturity: " << nzcis.maturityDate()
                            << "\n    rate:     " << datum.rate/100.0);
    }

    //===========================================================================================
    // now test the forecasting capability of the index.

    from = hz->referenceDate();
    to = hz->maxDate()-1*Months; // a bit of margin for adjustments
    Schedule testIndex = MakeSchedule().from(from).to(to)
                            .withTenor(1*Months)
                            .withCalendar(UnitedKingdom())
                            .withConvention(ModifiedFollowing);

    // we are testing UKRPI which is not interpolated
    Date bd = hz->baseDate();
    Real bf = ii->fixing(bd);
    for (const auto& d : testIndex) {
        Real z = hz->zeroRate(d, Period(0, Days));
        Real t = hz->dayCounter().yearFraction(bd, inflationPeriod(d, ii->frequency()).first);
        Real calc = bf * std::pow(1+z, t);
        if (t<=0)
            calc = ii->fixing(d,false); // still historical
        if (std::fabs(calc - ii->fixing(d,true)) > eps)
            BOOST_ERROR("inflation index does not forecast correctly"
                        << "\n    date:        " << d
                        << "\n    base date:   " << bd
                        << "\n    base fixing: " << bf
                        << "\n    expected:    " << calc
                        << "\n    forecast:    " << ii->fixing(d,true));
    }

    //===========================================================================================
    // Add a seasonality correction.  The curve should recalculate and still reprice the swaps.

    Date nextBaseDate = inflationPeriod(hz->baseDate(), ii->frequency()).second;
    Date seasonalityBaseDate(31, January, nextBaseDate.year());
    vector<Rate> seasonalityFactors = {
        1.003245,
        1.000000,
        0.999715,
        1.000495,
        1.000929,
        0.998687,
        0.995949,
        0.994682,
        0.995949,
        1.000519,
        1.003705,
        1.004186
    };

    ext::shared_ptr<MultiplicativePriceSeasonality> nonUnitSeasonality =
        ext::make_shared<MultiplicativePriceSeasonality>(seasonalityBaseDate, Monthly, seasonalityFactors);

    pZITS->setSeasonality(nonUnitSeasonality);

    for (const auto& datum: zcData) {
        ZeroCouponInflationSwap nzcis(Swap::Payer,
                                      1000000.0,
                                      evaluationDate,
                                      datum.date,
                                      calendar, bdc, dc,
                                      datum.rate/100.0,
                                      ii, observationLag,
                                      CPI::AsIndex);
        nzcis.setPricingEngine(engine);

        BOOST_CHECK_MESSAGE(std::fabs(nzcis.NPV()) < eps,
                            "zero-coupon inflation swap does not reprice to zero"
                            << "\n    NPV:      " << nzcis.NPV()
                            << "\n    maturity: " << nzcis.maturityDate()
                            << "\n    rate:     " << datum.rate);
    }

    // remove circular refernce
    hz.linkTo(ext::shared_ptr<ZeroInflationTermStructure>());
}

BOOST_AUTO_TEST_CASE(testZeroTermStructureWithLag) {
    BOOST_TEST_MESSAGE("Testing old-style zero inflation term structure with observation lag...");

    // try the Zero UK
    Calendar calendar = UnitedKingdom();
    BusinessDayConvention bdc = ModifiedFollowing;
    Date evaluationDate(13, August, 2007);
    evaluationDate = calendar.adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
    .withTenor(1*Months)
    .withCalendar(UnitedKingdom())
    .withConvention(ModifiedFollowing);

    Real fixData[] = {
        189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3};

    RelinkableHandle<ZeroInflationTermStructure> hz;
    auto ii = ext::make_shared<UKRPI>(hz);
    for (Size i=0; i<LENGTH(fixData); i++) {
        ii->addFixing(rpiSchedule[i], fixData[i]);
    }

    Handle<YieldTermStructure> nominalTS(nominalTermStructure());

    // now build the zero inflation curve
    std::vector<Datum> zcData = {
        { Date(13, August, 2008), 2.93 },
        { Date(13, August, 2009), 2.95 },
        { Date(13, August, 2010), 2.965 },
        { Date(15, August, 2011), 2.98 },
        { Date(13, August, 2012), 3.0 },
        { Date(13, August, 2014), 3.06 },
        { Date(13, August, 2017), 3.175 },
        { Date(13, August, 2019), 3.243 },
        { Date(15, August, 2022), 3.293 },
        { Date(14, August, 2027), 3.338 },
        { Date(13, August, 2032), 3.348 },
        { Date(15, August, 2037), 3.348 },
        { Date(13, August, 2047), 3.308 },
        { Date(13, August, 2057), 3.228 }
    };

    Period observationLag = Period(3, Months);
    DayCounter dc = Thirty360(Thirty360::BondBasis);
    Frequency frequency = Monthly;

    auto makeHelper = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<ZeroCouponInflationSwapHelper>(
            quote, observationLag, maturity, calendar, bdc, dc, ii, CPI::AsIndex, nominalTS);
    };
    auto helpers = makeHelpers<ZeroInflationTermStructure>(zcData, makeHelper);

    Rate baseZeroRate = zcData[0].rate/100.0;
    QL_DEPRECATED_DISABLE_WARNING
    ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITS(
                        new PiecewiseZeroInflationCurve<Linear>(
                        evaluationDate, calendar, dc, observationLag,
                        frequency, baseZeroRate, helpers));
    QL_DEPRECATED_ENABLE_WARNING
    hz.linkTo(pZITS);

    //===========================================================================================
    // first check that the quoted swaps are repriced correctly

    const Real eps = 1.0e-6;
    auto engine = ext::make_shared<DiscountingSwapEngine>(nominalTS);
    
    for (const auto& datum: zcData) {
        ZeroCouponInflationSwap nzcis(Swap::Payer,
                                      1000000.0,
                                      evaluationDate,
                                      datum.date,
                                      calendar, bdc, dc,
                                      datum.rate/100.0,
                                      ii, observationLag,
                                      CPI::AsIndex);
        nzcis.setPricingEngine(engine);

        BOOST_CHECK_MESSAGE(std::fabs(nzcis.NPV()) < eps,
                            "zero-coupon inflation swap does not reprice to zero"
                            << "\n    NPV:      " << nzcis.NPV()
                            << "\n    maturity: " << nzcis.maturityDate()
                            << "\n    rate:     " << datum.rate);
    }

    //===========================================================================================
    // now test the forecasting capability of the index.

    from = hz->referenceDate();
    to = hz->maxDate()-1*Months; // a bit of margin for adjustments
    Schedule testIndex = MakeSchedule().from(from).to(to)
                            .withTenor(1*Months)
                            .withCalendar(UnitedKingdom())
                            .withConvention(ModifiedFollowing);

    // we are testing UKRPI which is not interpolated
    Date bd = hz->baseDate();
    Real bf = ii->fixing(bd);
    for (const auto& d : testIndex) {
        Real z = hz->zeroRate(d, Period(0, Days));
        Real t = hz->dayCounter().yearFraction(bd, inflationPeriod(d, ii->frequency()).first);
        Real calc = bf * std::pow(1+z, t);
        if (t<=0)
            calc = ii->fixing(d,false); // still historical
        if (std::fabs(calc - ii->fixing(d,true)) > eps)
            BOOST_ERROR("inflation index does not forecast correctly"
                        << "\n    date:        " << d
                        << "\n    base date:   " << bd
                        << "\n    base fixing: " << bf
                        << "\n    expected:    " << calc
                        << "\n    forecast:    " << ii->fixing(d,true));
    }

    //===========================================================================================
    // Add a seasonality correction.  The curve should recalculate and still reprice the swaps.

    Date nextBaseDate = inflationPeriod(hz->baseDate(), ii->frequency()).second;
    Date seasonalityBaseDate(31, January, nextBaseDate.year());
    vector<Rate> seasonalityFactors = {
        1.003245,
        1.000000,
        0.999715,
        1.000495,
        1.000929,
        0.998687,
        0.995949,
        0.994682,
        0.995949,
        1.000519,
        1.003705,
        1.004186
    };

    ext::shared_ptr<MultiplicativePriceSeasonality> nonUnitSeasonality =
        ext::make_shared<MultiplicativePriceSeasonality>(seasonalityBaseDate, Monthly, seasonalityFactors);

    pZITS->setSeasonality(nonUnitSeasonality);
    
    for (const auto& datum: zcData) {
        ZeroCouponInflationSwap nzcis(Swap::Payer,
                                      1000000.0,
                                      evaluationDate,
                                      datum.date,
                                      calendar, bdc, dc,
                                      datum.rate/100.0,
                                      ii, observationLag,
                                      CPI::AsIndex);
        nzcis.setPricingEngine(engine);

        BOOST_CHECK_MESSAGE(std::fabs(nzcis.NPV()) < eps,
                            "zero-coupon inflation swap does not reprice to zero"
                            << "\n    NPV:      " << nzcis.NPV()
                            << "\n    maturity: " << nzcis.maturityDate()
                            << "\n    rate:     " << datum.rate);
    }

    // remove circular refernce
    hz.linkTo(ext::shared_ptr<ZeroInflationTermStructure>());
}

BOOST_AUTO_TEST_CASE(testSeasonalityCorrection) {
    BOOST_TEST_MESSAGE("Testing seasonality correction on zero inflation term structure...");

    // try the Zero UK
    Calendar calendar = UnitedKingdom();
    Date evaluationDate(13, August, 2007);
    evaluationDate = calendar.adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
    .withTenor(1*Months)
    .withCalendar(UnitedKingdom())
    .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3};

    RelinkableHandle<ZeroInflationTermStructure> hz;
    auto ii = ext::make_shared<UKRPI>(hz);
    for (Size i=0; i<LENGTH(fixData); i++) {
        ii->addFixing(rpiSchedule[i], fixData[i]);
    }

    ext::shared_ptr<YieldTermStructure> nominalTS = nominalTermStructure();

    std::vector<Date> nodes = {
        Date(1, June, 2007),
        Date(1, June, 2008),
        Date(1, June, 2009),
        Date(1, June, 2010),
        Date(1, June, 2011),
        Date(1, June, 2012),
        Date(1, June, 2014),
        Date(1, June, 2017),
        Date(1, June, 2019),
        Date(1, June, 2022),
        Date(1, June, 2027),
        Date(1, June, 2032),
        Date(1, June, 2037),
        Date(1, June, 2047),
        Date(1, June, 2057)
    };

    std::vector<Rate> rates = {
        0.0293,
        0.0293,
        0.0295,
        0.02965,
        0.0298,
        0.03,
        0.0306,
        0.03175,
        0.03243,
        0.03293,
        0.03338,
        0.03348,
        0.03348,
        0.03308,
        0.03228
    };

    DayCounter dc = Thirty360(Thirty360::BondBasis);
    Frequency frequency = Monthly;

    auto zeroCurve = ext::make_shared<InterpolatedZeroInflationCurve<Linear>>(
                                 evaluationDate, nodes, rates, frequency, dc);
    hz.linkTo(zeroCurve);

    // Perform checks on the seasonality for this non-interpolated index
    checkSeasonality(hz, ii);
}

BOOST_AUTO_TEST_CASE(testZeroIndexFutureFixing) {
    BOOST_TEST_MESSAGE("Testing that zero inflation indices forecast future fixings...");

    EUHICP euhicp;

    Date sample_date = Date(1,December,2013);
    Real sample_fixing = 117.48;
    euhicp.addFixing(sample_date, sample_fixing);

    // fixing date in the past
    Date evaluationDate = euhicp.fixingCalendar().adjust(sample_date + 2*Weeks);
    Settings::instance().evaluationDate() = evaluationDate;
    Real fixing = euhicp.fixing(sample_date);
    if (std::fabs(fixing - sample_fixing) > 1e-12)
        BOOST_ERROR("Failed to retrieve correct fixing: "
                    << "\n    returned: " << fixing
                    << "\n    expected: " << sample_fixing);

    // fixing date in the future
    evaluationDate = euhicp.fixingCalendar().adjust(sample_date - 2*Weeks);
    Settings::instance().evaluationDate() = evaluationDate;
    bool retrieved = false;
    try {
        fixing = euhicp.fixing(sample_date);
        // the above should throw for lack of a forecast curve, so
        // this shouldn't be executed and retrieved should stay false
        retrieved = true;
    } catch (Error&) {}

    if (retrieved)
        BOOST_ERROR("Retrieved future fixing: "
                    << "\n    returned: " << fixing);
}

BOOST_AUTO_TEST_CASE(testInterpolatedZeroTermStructure) {
    BOOST_TEST_MESSAGE("Testing interpolated zero-rate inflation curve...");

    Date today = Date(27, January, 2022);
    Settings::instance().evaluationDate() = today;

    Date baseDate = Date(1, December, 2021);

    std::vector<Date> dates = {
        baseDate,
        today + 7 * Days,
        today + 14 * Days,
        today + 1 * Months,
        today + 2 * Months,
        today + 3 * Months,
        today + 6 * Months,
        today + 1 * Years,
        today + 2 * Years,
        today + 5 * Years,
        today + 10 * Years
    };
    std::vector<Rate> rates = { 0.01, 0.01, 0.011, 0.012, 0.013, 0.015, 0.018, 0.02, 0.025, 0.03, 0.03 };

    auto curve = ext::make_shared<InterpolatedZeroInflationCurve<Linear>>(
        today, dates, rates, Monthly, Actual360());

    auto nodes = curve->nodes();

    BOOST_CHECK_MESSAGE(nodes.size() == dates.size(), "different number of nodes and input dates");

    for (Size i=0; i<dates.size(); ++i) {
        BOOST_CHECK_MESSAGE(dates[i] == nodes[i].first,
                            "node " << i << " at " << nodes[i].first << "; " << dates[i] << " expected");
    }
}

//===========================================================================================
// year on year tests, index, termstructure, and swaps
//===========================================================================================

BOOST_AUTO_TEST_CASE(testQuotedYYIndex) {
    BOOST_TEST_MESSAGE("Testing quoted year-on-year inflation indices...");

    YYEUHICP yyeuhicp(true);
    if (yyeuhicp.name() != "EU YY_HICP"
        || yyeuhicp.frequency() != Monthly
        || yyeuhicp.revised()
        || !yyeuhicp.interpolated()
        || yyeuhicp.ratio()
        || yyeuhicp.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong year-on-year EU HICP data ("
                    << yyeuhicp.name() << ", "
                    << yyeuhicp.frequency() << ", "
                    << yyeuhicp.revised() << ", "
                    << yyeuhicp.interpolated() << ", "
                    << yyeuhicp.ratio() << ", "
                    << yyeuhicp.availabilityLag() << ")");
    }

    YYUKRPI yyukrpi(false);
    if (yyukrpi.name() != "UK YY_RPI"
        || yyukrpi.frequency() != Monthly
        || yyukrpi.revised()
        || yyukrpi.interpolated()
        || yyukrpi.ratio()
        || yyukrpi.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong year-on-year UK RPI data ("
                    << yyukrpi.name() << ", "
                    << yyukrpi.frequency() << ", "
                    << yyukrpi.revised() << ", "
                    << yyukrpi.interpolated() << ", "
                    << yyukrpi.ratio() << ", "
                    << yyukrpi.availabilityLag() << ")");
    }
}

BOOST_AUTO_TEST_CASE(testRatioYYIndex) {
    BOOST_TEST_MESSAGE("Testing ratio year-on-year inflation indices...");

    auto euhicp = ext::make_shared<EUHICP>();
    auto ukrpi = ext::make_shared<UKRPI>();

    YoYInflationIndex yyeuhicpr(euhicp, true);
    if (yyeuhicpr.name() != "EU YYR_HICP"
        || yyeuhicpr.frequency() != Monthly
        || yyeuhicpr.revised()
        || !yyeuhicpr.interpolated()
        || !yyeuhicpr.ratio()
        || yyeuhicpr.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong year-on-year EU HICPr data ("
                    << yyeuhicpr.name() << ", "
                    << yyeuhicpr.frequency() << ", "
                    << yyeuhicpr.revised() << ", "
                    << yyeuhicpr.interpolated() << ", "
                    << yyeuhicpr.ratio() << ", "
                    << yyeuhicpr.availabilityLag() << ")");
    }

    YoYInflationIndex yyukrpir(ukrpi, false);
    if (yyukrpir.name() != "UK YYR_RPI"
        || yyukrpir.frequency() != Monthly
        || yyukrpir.revised()
        || yyukrpir.interpolated()
        || !yyukrpir.ratio()
        || yyukrpir.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong year-on-year UK RPIr data ("
                    << yyukrpir.name() << ", "
                    << yyukrpir.frequency() << ", "
                    << yyukrpir.revised() << ", "
                    << yyukrpir.interpolated() << ", "
                    << yyukrpir.ratio() << ", "
                    << yyukrpir.availabilityLag() << ")");
    }


    // Retrieval test.
    //----------------
    // make sure of the evaluation date
    Date evaluationDate(13, August, 2007);
    evaluationDate = UnitedKingdom().adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
    .withTenor(1*Months)
    .withCalendar(UnitedKingdom())
    .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3 };

    for (Size i=0; i<LENGTH(fixData);i++) {
        ukrpi->addFixing(rpiSchedule[i], fixData[i]);
    }

    auto iir = ext::make_shared<YoYInflationIndex>(ukrpi, false);
    auto iirYES = ext::make_shared<YoYInflationIndex>(ukrpi, true);

    Date todayMinusLag = evaluationDate - iir->availabilityLag();
    std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, iir->frequency());
    todayMinusLag = lim.second + 1 - 2*Period(iir->frequency());

    Real eps = 1.0e-8;

    // Interpolation tests
    //--------------------
    // (no TS so can't forecast).
    for (Size i=13; i<rpiSchedule.size();i++) {
        std::pair<Date,Date> lim = inflationPeriod(rpiSchedule[i],
                                                   iir->frequency());
        std::pair<Date,Date> limBef = inflationPeriod(rpiSchedule[i-12],
                                                      iir->frequency());
        for (Date d=lim.first; d<=lim.second; d++) {
            if (d < todayMinusLag) {
                Rate expected = fixData[i]/fixData[i-12] - 1.0;
                Rate calculated = iir->fixing(d);
                BOOST_CHECK_MESSAGE(std::fabs(calculated - expected) < eps,
                                    "Non-interpolated fixings not constant within a period: "
                                    << calculated
                                    << ", should be "
                                    << expected);

                Real dp= lim.second + 1- lim.first;
                Real dpBef=limBef.second + 1 - limBef.first;
                Real dl = d-lim.first;
                // potentially does not work on 29th Feb
                Real dlBef = NullCalendar().advance(d, -1*Years, ModifiedFollowing)
                -limBef.first;

                Real linearNow = fixData[i] + (fixData[i+1]-fixData[i])*dl/dp;
                Real linearBef = fixData[i-12] + (fixData[i+1-12]-fixData[i-12])*dlBef/dpBef;
                Rate expectedYES = linearNow / linearBef - 1.0;
                Rate calculatedYES = iirYES->fixing(d);
                BOOST_CHECK_MESSAGE(fabs(expectedYES-calculatedYES)<eps,
                                    "Error in interpolated fixings: expect "<<expectedYES
                                    <<" see " << calculatedYES
                                    <<" flat " << calculated
                                    <<", data: "<< fixData[i-12] <<", "<< fixData[i+1-12]
                                    <<", "<<    fixData[i] <<", "<< fixData[i+1]
                                    <<", fac: "<< dp <<", "<< dl
                                    <<", "<< dpBef <<", "<< dlBef
                                    <<", to: "<<linearNow<<", "<<linearBef
                                    );
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testOldRatioYYIndex) {
    BOOST_TEST_MESSAGE("Testing old-style ratio year-on-year inflation indices...");

    QL_DEPRECATED_DISABLE_WARNING

    YYEUHICPr yyeuhicpr(true);

    QL_DEPRECATED_ENABLE_WARNING

    if (yyeuhicpr.name() != "EU YYR_HICP"
        || yyeuhicpr.frequency() != Monthly
        || yyeuhicpr.revised()
        || !yyeuhicpr.interpolated()
        || !yyeuhicpr.ratio()
        || yyeuhicpr.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong year-on-year EU HICPr data ("
                    << yyeuhicpr.name() << ", "
                    << yyeuhicpr.frequency() << ", "
                    << yyeuhicpr.revised() << ", "
                    << yyeuhicpr.interpolated() << ", "
                    << yyeuhicpr.ratio() << ", "
                    << yyeuhicpr.availabilityLag() << ")");
    }

    QL_DEPRECATED_DISABLE_WARNING

    YYUKRPIr yyukrpir(false);

    QL_DEPRECATED_ENABLE_WARNING

    if (yyukrpir.name() != "UK YYR_RPI"
        || yyukrpir.frequency() != Monthly
        || yyukrpir.revised()
        || yyukrpir.interpolated()
        || !yyukrpir.ratio()
        || yyukrpir.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong year-on-year UK RPIr data ("
                    << yyukrpir.name() << ", "
                    << yyukrpir.frequency() << ", "
                    << yyukrpir.revised() << ", "
                    << yyukrpir.interpolated() << ", "
                    << yyukrpir.ratio() << ", "
                    << yyukrpir.availabilityLag() << ")");
    }


    // Retrieval test.
    //----------------
    // make sure of the evaluation date
    Date evaluationDate(13, August, 2007);
    evaluationDate = UnitedKingdom().adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
    .withTenor(1*Months)
    .withCalendar(UnitedKingdom())
    .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3 };

    bool interp = false;

    QL_DEPRECATED_DISABLE_WARNING

    ext::shared_ptr<YYUKRPIr> iir(new YYUKRPIr(interp));
    ext::shared_ptr<YYUKRPIr> iirYES(new YYUKRPIr(true));

    QL_DEPRECATED_ENABLE_WARNING

    for (Size i=0; i<LENGTH(fixData);i++) {
        iir->addFixing(rpiSchedule[i], fixData[i]);
        iirYES->addFixing(rpiSchedule[i], fixData[i]);
    }

    Date todayMinusLag = evaluationDate - iir->availabilityLag();
    std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, iir->frequency());
    todayMinusLag = lim.second + 1 - 2*Period(iir->frequency());

    Real eps = 1.0e-8;

    // Interpolation tests
    //--------------------
    // (no TS so can't forecast).
    for (Size i=13; i<rpiSchedule.size();i++) {
        std::pair<Date,Date> lim = inflationPeriod(rpiSchedule[i],
                                                   iir->frequency());
        std::pair<Date,Date> limBef = inflationPeriod(rpiSchedule[i-12],
                                                      iir->frequency());
        for (Date d=lim.first; d<=lim.second; d++) {
            if (d < todayMinusLag) {
                Rate expected = fixData[i]/fixData[i-12] - 1.0;
                Rate calculated = iir->fixing(d);
                BOOST_CHECK_MESSAGE(std::fabs(calculated - expected) < eps,
                                    "Non-interpolated fixings not constant within a period: "
                                    << calculated
                                    << ", should be "
                                    << expected);

                Real dp= lim.second + 1- lim.first;
                Real dpBef=limBef.second + 1 - limBef.first;
                Real dl = d-lim.first;
                // potentially does not work on 29th Feb
                Real dlBef = NullCalendar().advance(d, -1*Years, ModifiedFollowing)
                -limBef.first;

                Real linearNow = fixData[i] + (fixData[i+1]-fixData[i])*dl/dp;
                Real linearBef = fixData[i-12] + (fixData[i+1-12]-fixData[i-12])*dlBef/dpBef;
                Rate expectedYES = linearNow / linearBef - 1.0;
                Rate calculatedYES = iirYES->fixing(d);
                BOOST_CHECK_MESSAGE(fabs(expectedYES-calculatedYES)<eps,
                                    "Error in interpolated fixings: expect "<<expectedYES
                                    <<" see " << calculatedYES
                                    <<" flat " << calculated
                                    <<", data: "<< fixData[i-12] <<", "<< fixData[i+1-12]
                                    <<", "<<    fixData[i] <<", "<< fixData[i+1]
                                    <<", fac: "<< dp <<", "<< dl
                                    <<", "<< dpBef <<", "<< dlBef
                                    <<", to: "<<linearNow<<", "<<linearBef
                                    );
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testYYTermStructure) {
    BOOST_TEST_MESSAGE("Testing year-on-year inflation term structure...");

    // try the YY UK
    Calendar calendar = UnitedKingdom();
    BusinessDayConvention bdc = ModifiedFollowing;
    Date evaluationDate(13, August, 2007);
    evaluationDate = calendar.adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;


    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
    .withTenor(1*Months)
    .withCalendar(UnitedKingdom())
    .withConvention(ModifiedFollowing);
    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3 };

    RelinkableHandle<YoYInflationTermStructure> hy;
    bool interp = false;
    auto rpi = ext::make_shared<UKRPI>();
    auto iir = ext::make_shared<YoYInflationIndex>(rpi, interp, hy);
    for (Size i=0; i<LENGTH(fixData); i++) {
        rpi->addFixing(rpiSchedule[i], fixData[i]);
    }



    ext::shared_ptr<YieldTermStructure> nominalTS = nominalTermStructure();

    // now build the YoY inflation curve
    std::vector<Datum> yyData = {
        { Date(13, August, 2008), 2.95 },
        { Date(13, August, 2009), 2.95 },
        { Date(13, August, 2010), 2.93 },
        { Date(15, August, 2011), 2.955 },
        { Date(13, August, 2012), 2.945 },
        { Date(13, August, 2013), 2.985 },
        { Date(13, August, 2014), 3.01 },
        { Date(13, August, 2015), 3.035 },
        { Date(13, August, 2016), 3.055 },  // note that
        { Date(13, August, 2017), 3.075 },  // some dates will be on
        { Date(13, August, 2019), 3.105 },  // holidays but the payment
        { Date(15, August, 2022), 3.135 },  // calendar will roll them
        { Date(13, August, 2027), 3.155 },
        { Date(13, August, 2032), 3.145 },
        { Date(13, August, 2037), 3.145 }
    };

    Period observationLag = Period(2,Months);
    DayCounter dc = Thirty360(Thirty360::BondBasis);

    // now build the helpers ...
    auto makeHelper = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<YearOnYearInflationSwapHelper>(
            quote, observationLag, maturity, calendar, bdc, dc, iir,
            Handle<YieldTermStructure>(nominalTS));
    };
    auto helpers = makeHelpers<YoYInflationTermStructure>(yyData, makeHelper);

    Rate baseYYRate = yyData[0].rate/100.0;
    ext::shared_ptr<PiecewiseYoYInflationCurve<Linear> > pYYTS(
        new PiecewiseYoYInflationCurve<Linear>(
                evaluationDate, calendar, dc, observationLag,
                iir->frequency(),iir->interpolated(), baseYYRate,
                helpers));
    pYYTS->recalculate();

    // validation
    // yoy swaps should reprice to zero
    // yy rates should not equal yySwap rates
    Real eps = 0.000001;
    // usual swap engine
    Handle<YieldTermStructure> hTS(nominalTS);
    ext::shared_ptr<PricingEngine> sppe(new DiscountingSwapEngine(hTS));

    // make sure that the index has the latest yoy term structure
    hy.linkTo(pYYTS);

    for (Size j = 1; j < yyData.size(); j++) {

        from = nominalTS->referenceDate();
        to = yyData[j].date;
        Schedule yoySchedule = MakeSchedule().from(from).to(to)
        .withConvention(Unadjusted) // fixed leg gets calendar from
        .withCalendar(calendar)     // schedule
        .withTenor(1*Years)
        .backwards()
        ;

        YearOnYearInflationSwap yyS2(Swap::Payer,
                                     1000000.0,
                                     yoySchedule,//fixed schedule, but same as yoy
                                     yyData[j].rate/100.0,
                                     dc,
                                     yoySchedule,
                                     iir,
                                     observationLag,
                                     0.0,        //spread on index
                                     dc,
                                     UnitedKingdom());

        yyS2.setPricingEngine(sppe);

        BOOST_CHECK_MESSAGE(fabs(yyS2.NPV())<eps,"fresh yoy swap NPV!=0 from TS "
                <<"swap quote for pt " << j
                << ", is " << yyData[j].rate/100.0
                <<" vs YoY rate "<< pYYTS->yoyRate(yyData[j].date-observationLag)
                <<" at quote date "<<(yyData[j].date-observationLag)
                <<", NPV of a fresh yoy swap is " << yyS2.NPV()
                <<"\n      fair rate " << yyS2.fairRate()
                <<" payment "<<yyS2.paymentConvention());
    }

    Size jj=3;
    for (Size k = 0; k < 14; k++) {

        from = nominalTS->referenceDate() - k*Months;
        to = yyData[jj].date - k*Months;
        Schedule yoySchedule = MakeSchedule().from(from).to(to)
        .withConvention(Unadjusted) // fixed leg gets calendar from
        .withCalendar(calendar)     // schedule
        .withTenor(1*Years)
        .backwards()
        ;

        YearOnYearInflationSwap yyS3(Swap::Payer,
                                     1000000.0,
                                     yoySchedule,//fixed schedule, but same as yoy
                                     yyData[jj].rate/100.0,
                                     dc,
                                     yoySchedule,
                                     iir,
                                     observationLag,
                                     0.0,        //spread on index
                                     dc,
                                     UnitedKingdom());

        yyS3.setPricingEngine(sppe);

        BOOST_CHECK_MESSAGE(fabs(yyS3.NPV())< 20000.0,
                            "unexpected size of aged YoY swap, aged "
                            <<k<<" months: YY aged NPV = " << yyS3.NPV()
                            <<", legs "<< yyS3.legNPV(0) << " and " << yyS3.legNPV(1)
                            );
    }
    // remove circular refernce
    hy.linkTo(ext::shared_ptr<YoYInflationTermStructure>());
}

BOOST_AUTO_TEST_CASE(testPeriod) {
    BOOST_TEST_MESSAGE("Testing inflation period...");

    Date d;
    Frequency f;
    std::pair<Date,Date> res;
    int days[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

    for (int year = 1950; year < 2051; ++year) {

        if (Date::isLeap(year))
            days[2] = 29;
        else
            days[2] = 28;

        for (Size i=1; i<=12; ++i){

            d = Date(1,Month(i),year);

            f = Monthly;
            res = inflationPeriod (d,f);
            if (res.first != Date(1,Month(i),year)
                || res.second != Date(days[i],Month(i),year)) {
                REPORT_FAILURE(d, res, "Monthly");
            }

            f = Quarterly;
            res = inflationPeriod (d,f);

            if ( (i==1 || i==2 || i==3) &&
                (res.first != Date(1,Month(1),year)
                 || res.second != Date(31,Month(3),year))) {
                REPORT_FAILURE(d, res, "Quarterly");
            }
            else if ( (i==4 || i==5 || i==6) &&
                (res.first != Date(1,Month(4),year)
                 || res.second != Date(30,Month(6),year))) {
                REPORT_FAILURE(d, res, "Quarterly");
            }
            else if ( (i==7 || i==8 || i==9) &&
                (res.first != Date(1,Month(7),year)
                 || res.second != Date(30,Month(9),year))) {
                REPORT_FAILURE(d, res, "Quarterly");
            }
            else if ( (i==10 || i==11 || i==12) &&
                (res.first != Date(1,Month(10),year)
                 || res.second != Date(31,Month(12),year))) {
                REPORT_FAILURE(d, res, "Quarterly");
            }

            f = Semiannual;
            res = inflationPeriod (d,f);

            if ( (i>0 && i<7) && (
                res.first != Date(1,Month(1),year)
                || res.second != Date(30,Month(6),year))) {
                REPORT_FAILURE(d, res, "Semiannual");
            }
            else if ( (i>6 && i<13) && (
                res.first != Date(1,Month(7),year)
                || res.second != Date(31,Month(12),year))) {
                REPORT_FAILURE(d, res, "Semiannual");
            }

            f = Annual;
            res = inflationPeriod (d,f);

            if (res.first != Date(1,Month(1),year)
                || res.second != Date(31,Month(12),year)) {
                REPORT_FAILURE(d, res, "Annual");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testCpiFlatInterpolation) {
    BOOST_TEST_MESSAGE("Testing CPI flat interpolation...");

    Settings::instance().evaluationDate() = Date(10, February, 2022);

    auto testIndex = ext::make_shared<UKRPI>();

    testIndex->addFixing(Date(1, November, 2020), 293.5);
    testIndex->addFixing(Date(1, December, 2020), 295.4);
    testIndex->addFixing(Date(1, January,  2021), 294.6);
    testIndex->addFixing(Date(1, February, 2021), 296.0);
    testIndex->addFixing(Date(1, March,    2021), 296.9);

    Real calculated = CPI::laggedFixing(testIndex, Date(10, February, 2021), 3 * Months, CPI::Flat);
    Real expected = 293.5;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex, Date(12, May, 2021), 3 * Months, CPI::Flat);
    expected = 296.0;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex, Date(25, June, 2021), 3 * Months, CPI::Flat);
    expected = 296.9;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);
}

BOOST_AUTO_TEST_CASE(testCpiLinearInterpolation) {
    BOOST_TEST_MESSAGE("Testing CPI linear interpolation...");

    Settings::instance().evaluationDate() = Date(10, February, 2022);

    auto testIndex = ext::make_shared<UKRPI>();

    testIndex->addFixing(Date(1, November, 2020), 293.5);
    testIndex->addFixing(Date(1, December, 2020), 295.4);
    testIndex->addFixing(Date(1, January,  2021), 294.6);
    testIndex->addFixing(Date(1, February, 2021), 296.0);
    testIndex->addFixing(Date(1, March,    2021), 296.9);

    Real calculated = CPI::laggedFixing(testIndex, Date(10, February, 2021), 3 * Months, CPI::Linear);
    Real expected = 293.5 * (19/28.0) + 295.4 * (9/28.0);

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex, Date(12, May, 2021), 3 * Months, CPI::Linear);
    expected = 296.0 * (20/31.0) + 296.9 * (11/31.0);

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    // this would require April's fixing
    BOOST_CHECK_THROW(
        calculated = CPI::laggedFixing(testIndex, Date(25, June, 2021), 3 * Months, CPI::Linear),
        Error);

    // however, this is a special case
    calculated = CPI::laggedFixing(testIndex, Date(1, June, 2021), 3 * Months, CPI::Linear);
    expected = 296.9;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);
}

BOOST_AUTO_TEST_CASE(testCpiAsIndexInterpolation) {
    BOOST_TEST_MESSAGE("Testing CPI as-index interpolation...");

    Date today = Date(10, February, 2022);
    Settings::instance().evaluationDate() = today;

    // AsIndex requires a term structure, even for fixings in the past
    std::vector<Date> dates = { today - 3*Months, today + 5*Years };
    std::vector<Rate> rates = { 0.02, 0.02 };
    Handle<ZeroInflationTermStructure> mock_curve(
            ext::make_shared<ZeroInflationCurve>(today, dates, rates, Monthly, Actual360()));
    auto testIndex = ext::make_shared<UKRPI>(mock_curve);

    testIndex->addFixing(Date(1, November, 2020), 293.5);
    testIndex->addFixing(Date(1, December, 2020), 295.4);
    testIndex->addFixing(Date(1, January,  2021), 294.6);
    testIndex->addFixing(Date(1, February, 2021), 296.0);
    testIndex->addFixing(Date(1, March,    2021), 296.9);

    Real calculated = CPI::laggedFixing(testIndex, Date(10, February, 2021), 3 * Months, CPI::AsIndex);
    Real expected = 293.5;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex, Date(12, May, 2021), 3 * Months, CPI::AsIndex);
    expected = 296.0;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex, Date(25, June, 2021), 3 * Months, CPI::AsIndex);
    expected = 296.9;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
