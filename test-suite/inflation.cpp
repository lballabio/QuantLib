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

#include "inflation.hpp"
#include "utilities.hpp"

#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
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

#undef REPORT_FAILURE
#define REPORT_FAILURE(d, res, periodName) \
    BOOST_ERROR("wrong " << periodName << " inflation period for Date (1 " \
        << d << "), Start Date ( " \
        << res.first << "), End Date (" \
        << res.second << ")"); \

namespace inflation_test {

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

        QL_DEPRECATED_DISABLE_WARNING_III

        std::vector<ext::shared_ptr<BootstrapHelper<T> > > instruments;
        for (Datum datum : iiData) {
            Date maturity = datum.date;
            Handle<Quote> quote(ext::shared_ptr<Quote>(new SimpleQuote(datum.rate / 100.0)));
            auto anInstrument = makeHelper(quote, maturity);
            instruments.push_back(anInstrument);
        }

        return instruments;

        QL_DEPRECATED_ENABLE_WARNING_III
    }
}

//===========================================================================================
// zero inflation tests, index, termstructure, and swaps
//===========================================================================================

namespace inflation_test {

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
        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_METHOD
        if (!ii->interpolated()) {
        QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_METHOD
            expectedSeasonalityFixings[i] =
                ii->fixing(fixingDates[i], true) * seasonalityFactors[i] / baseSeasonality;
        } else {
            std::pair<Date, Date> p1 = inflationPeriod(fixingDates[i], ii->frequency());
            Date firstDayCurrentPeriod = p1.first;
            Date firstDayNextPeriod = p1.second + 1;
            Month firstMonth = firstDayCurrentPeriod.month();
            Month secondMonth = firstDayNextPeriod.month();
            Size firstMonthIndex = static_cast<Size>(firstMonth) - 1;
            Size secondMonthIndex = static_cast<Size>(secondMonth) - 1;

            Period observationLag = ii->zeroInflationTermStructure()->observationLag();
            Date observationDate = fixingDates[i] + observationLag;
            std::pair<Date, Date> p2 = inflationPeriod(observationDate, ii->frequency());
            Real daysInPeriod = (p2.second + 1) - p2.first;
            Real interpolationCoefficient = (observationDate - p2.first) / daysInPeriod;

            Rate i1adj = ii->fixing(firstDayCurrentPeriod, true) *
                         seasonalityFactors[firstMonthIndex] / baseSeasonality;

            Rate i2adj = ii->fixing(firstDayNextPeriod, true) *
                         seasonalityFactors[secondMonthIndex] / baseSeasonality;
            expectedSeasonalityFixings[i] =
                i1adj + (i2adj - i1adj) * interpolationCoefficient;
        }
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
    hz->setSeasonality();
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

}

void InflationTest::testZeroIndex() {
    BOOST_TEST_MESSAGE("Testing zero inflation indices...");

    QL_DEPRECATED_DISABLE_WARNING_III_CONSTRUCTOR

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    EUHICP euhicp(true);
    if (euhicp.name() != "EU HICP"
        || euhicp.frequency() != Monthly
        || euhicp.revised()
        || !euhicp.interpolated()
        || euhicp.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong EU HICP data ("
                    << euhicp.name() << ", "
                    << euhicp.frequency() << ", "
                    << euhicp.revised() << ", "
                    << euhicp.interpolated() << ", "
                    << euhicp.availabilityLag() << ")");
    }

    UKRPI ukrpi(false);
    if (ukrpi.name() != "UK RPI"
        || ukrpi.frequency() != Monthly
        || ukrpi.revised()
        || ukrpi.interpolated()
        || ukrpi.availabilityLag() != 1*Months) {
        BOOST_ERROR("wrong UK RPI data ("
                    << ukrpi.name() << ", "
                    << ukrpi.frequency() << ", "
                    << ukrpi.revised() << ", "
                    << ukrpi.interpolated() << ", "
                    << ukrpi.availabilityLag() << ")");
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

    bool interp = false;
    ext::shared_ptr<UKRPI> iir(new UKRPI(interp));
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

    QL_DEPRECATED_ENABLE_WARNING_III_CONSTRUCTOR
}



void InflationTest::testZeroTermStructure() {
    BOOST_TEST_MESSAGE("Testing zero inflation term structure...");

    QL_DEPRECATED_DISABLE_WARNING_III_CONSTRUCTOR

    using namespace inflation_test;

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

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

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3};

    RelinkableHandle<ZeroInflationTermStructure> hz;
    bool interp = false;
    ext::shared_ptr<UKRPI> iiUKRPI(new UKRPI(interp, hz));
    for (Size i=0; i<LENGTH(fixData); i++) {
        iiUKRPI->addFixing(rpiSchedule[i], fixData[i]);
    }

    ext::shared_ptr<ZeroInflationIndex> ii = ext::dynamic_pointer_cast<ZeroInflationIndex>(iiUKRPI);
    ext::shared_ptr<YieldTermStructure> nominalTS = nominalTermStructure();

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

    Period observationLag = Period(2,Months);
    DayCounter dc = Thirty360(Thirty360::BondBasis);
    Frequency frequency = Monthly;

    auto makeHelper = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<ZeroCouponInflationSwapHelper>(
            quote, observationLag, maturity, calendar, bdc, dc, ii, CPI::AsIndex,
            Handle<YieldTermStructure>(nominalTS));
    };
    auto helpers = makeHelpers<ZeroInflationTermStructure>(zcData, makeHelper);

    Rate baseZeroRate = zcData[0].rate/100.0;
    ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITS(
                        new PiecewiseZeroInflationCurve<Linear>(
                        evaluationDate, calendar, dc, observationLag,
                        frequency, baseZeroRate, helpers));
    pZITS->recalculate();

    // first check that the zero rates on the curve match the data
    // and that the helpers give the correct implied rates
    const Real eps = 1.0e-8;
    bool forceLinearInterpolation = false;
    for (Size i=0; i<zcData.size(); i++) {
        BOOST_REQUIRE_MESSAGE(std::fabs(zcData[i].rate/100.0
            - pZITS->zeroRate(zcData[i].date, observationLag, forceLinearInterpolation)) < eps,
            "ZITS zeroRate != instrument "
            << pZITS->zeroRate(zcData[i].date, observationLag, forceLinearInterpolation)
            << " vs " << zcData[i].rate/100.0
            << " interpolation: " << ii->interpolated()
            << " forceLinearInterpolation " << forceLinearInterpolation);
        BOOST_REQUIRE_MESSAGE(std::fabs(helpers[i]->impliedQuote()
            - zcData[i].rate/100.0) < eps,
            "ZITS implied quote != instrument "
            << helpers[i]->impliedQuote()
            << " vs " << zcData[i].rate/100.0);
    }


    // now test the forecasting capability of the index.
    hz.linkTo(pZITS);
    from = hz->baseDate();
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
        Real t = hz->dayCounter().yearFraction(bd, d);
        if(!ii->interpolated()) // because fixing constant over period
            t = hz->dayCounter().yearFraction(bd,
                inflationPeriod(d, ii->frequency()).first);
        Real calc = bf * pow( 1+z, t);
        if (t<=0)
            calc = ii->fixing(d,false); // still historical
        if (std::fabs(calc - ii->fixing(d,true))/10000.0 > eps)
            BOOST_ERROR("ZC index does not forecast correctly for date " << d
                        << " from base date " << bd
                        << " with fixing " << bf
                        << ", correct:  " << calc
                        << ", fix: " << ii->fixing(d,true)
                        << ", t " << t);
    }


    //===========================================================================================
    // Test zero-inflation-indexed (i.e. cpi ratio) cashflow
    // just ordinary indexed cashflow with a zero inflation index

    Date baseDate(1, January, 2006);
    Date fixDate(1, August, 2014);
    Date payDate=UnitedKingdom().adjust(fixDate+Period(3,Months),ModifiedFollowing);
    ext::shared_ptr<Index> ind = ext::dynamic_pointer_cast<Index>(ii);
    BOOST_REQUIRE_MESSAGE(ind,"dynamic_pointer_cast to Index from InflationIndex failed");

    Real notional = 1000000.0;//1m
    IndexedCashFlow iicf(notional,ind,baseDate,fixDate,payDate);
    Real correctIndexed = ii->fixing(iicf.fixingDate())/ii->fixing(iicf.baseDate());
    Real calculatedIndexed = iicf.amount()/iicf.notional();
    BOOST_REQUIRE_MESSAGE(std::fabs(correctIndexed - calculatedIndexed) < eps,
               "IndexedCashFlow indexing wrong: " << calculatedIndexed << " vs correct = "
               << correctIndexed);


    //===========================================================================================
    // Test zero coupon swap

    // first make one ...

    ext::shared_ptr<ZeroInflationIndex> zii = ext::dynamic_pointer_cast<ZeroInflationIndex>(ii);
    BOOST_REQUIRE_MESSAGE(zii,"dynamic_pointer_cast to ZeroInflationIndex from UKRPI failed");
    ZeroCouponInflationSwap nzcis(Swap::Payer,
                                  1000000.0,
                                  evaluationDate,
                                  zcData[6].date,    // end date = maturity
                                  calendar, bdc, dc, zcData[6].rate/100.0, // fixed rate
                                  zii, observationLag, CPI::AsIndex);

    // N.B. no coupon pricer because it is not a coupon, effect of inflation curve via
    //      inflation curve attached to the inflation index.
    Handle<YieldTermStructure> hTS(nominalTS);
    ext::shared_ptr<PricingEngine> sppe(new DiscountingSwapEngine(hTS));
    nzcis.setPricingEngine(sppe);

    // ... and price it, should be zero
    BOOST_CHECK_MESSAGE(fabs(nzcis.NPV())<0.00001,"ZCIS does not reprice to zero "
                        << nzcis.NPV()
                        << evaluationDate << " to " << zcData[6].date << " becoming " << nzcis.maturityDate()
                        << " rate " << zcData[6].rate
                        << " fixed leg " << nzcis.legNPV(0)
                        << " indexed-predicted inflated leg " << nzcis.legNPV(1)
                        << " discount " << nominalTS->discount(nzcis.maturityDate())
                        );

    // Perform checks on the seasonality for this non-interpolated index
    checkSeasonality(hz, ii);

    //==============================================================================
    // now do an INTERPOLATED index, i.e. repeat everything on a fake version of
    // UKRPI (to save making another term structure)

    bool interpYES = true;
    ext::shared_ptr<UKRPI> iiUKRPIyes(new UKRPI(interpYES, hz));
    for (Size i=0; i<LENGTH(fixData);i++) {
        iiUKRPIyes->addFixing(rpiSchedule[i], fixData[i]);
    }

    ext::shared_ptr<ZeroInflationIndex> iiyes
        = ext::dynamic_pointer_cast<ZeroInflationIndex>(iiUKRPIyes);

    // now build the zero inflation curve
    // same data, bigger lag or it will be a self-contradiction
    Period observationLagyes = Period(3,Months);

    auto makeHelperYes = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<ZeroCouponInflationSwapHelper>(
            quote, observationLagyes, maturity, calendar, bdc, dc, iiyes, CPI::AsIndex,
            Handle<YieldTermStructure>(nominalTS));
    };
    auto helpersyes = makeHelpers<ZeroInflationTermStructure>(zcData, makeHelperYes);

    ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITSyes(
            new PiecewiseZeroInflationCurve<Linear>(
            evaluationDate, calendar, dc, observationLagyes,
            frequency, baseZeroRate, helpersyes));

    pZITSyes->recalculate();

    // For interpolated fixings, the zero rates on the curve no longer match the data.
    // The helpers still give the correct implied rates, of course.
    for (Size i=0; i<zcData.size(); i++) {
        BOOST_CHECK_MESSAGE(std::fabs(helpersyes[i]->impliedQuote()
                        - zcData[i].rate/100.0) < eps,
                    "ZITS INTERPOLATED implied quote != instrument "
                    << helpersyes[i]->impliedQuote()
                    << " vs " << zcData[i].rate/100.0);
    }


    //===========================================================================================
    // Test zero coupon swap

    hz.linkTo(pZITSyes);

    ext::shared_ptr<ZeroInflationIndex> ziiyes = ext::dynamic_pointer_cast<ZeroInflationIndex>(iiyes);
    BOOST_REQUIRE_MESSAGE(ziiyes,"dynamic_pointer_cast to ZeroInflationIndex from UKRPI-I failed");
    ZeroCouponInflationSwap nzcisyes(Swap::Payer,
                                     1000000.0,
                                     evaluationDate,
                                     zcData[6].date,    // end date = maturity
                                     calendar, bdc, dc, zcData[6].rate/100.0, // fixed rate
                                     ziiyes, observationLagyes, CPI::AsIndex);

    // N.B. no coupon pricer because it is not a coupon, effect of inflation curve via
    //      inflation curve attached to the inflation index.
    nzcisyes.setPricingEngine(sppe);

    // ... and price it, should be zero
    BOOST_CHECK_MESSAGE(fabs(nzcisyes.NPV())<0.00001,"ZCIS-I does not reprice to zero "
                        << nzcisyes.NPV()
                        << evaluationDate << " to " << zcData[6].date << " becoming " << nzcisyes.maturityDate()
                        << " rate " << zcData[6].rate
                        << " fixed leg " << nzcisyes.legNPV(0)
                        << " indexed-predicted inflated leg " << nzcisyes.legNPV(1)
                        << " discount " << nominalTS->discount(nzcisyes.maturityDate())
                        );

    // Perform checks on the seasonality for this interpolated index
    checkSeasonality(hz, iiyes);

    // remove circular refernce
    hz.linkTo(ext::shared_ptr<ZeroInflationTermStructure>());

    QL_DEPRECATED_ENABLE_WARNING_III_CONSTRUCTOR
}

void InflationTest::testZeroIndexFutureFixing() {
    BOOST_TEST_MESSAGE("Testing that zero inflation indices forecast future fixings...");

    QL_DEPRECATED_DISABLE_WARNING_III_CONSTRUCTOR

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    EUHICP euhicp(false);

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

    QL_DEPRECATED_ENABLE_WARNING_III_CONSTRUCTOR
}

void InflationTest::testInterpolatedZeroTermStructure() {
    BOOST_TEST_MESSAGE("Testing interpolated zero-rate inflation curve...");

    SavedSettings backup;

    Date today = Date(27, January, 2022);
    Settings::instance().evaluationDate() = today;

    Period lag = 3 * Months;

    std::vector<Date> dates = {
        today - lag,
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
        today, TARGET(), Actual360(), lag, Monthly, dates, rates);

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

void InflationTest::testYYIndex() {
    BOOST_TEST_MESSAGE("Testing year-on-year inflation indices...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

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

    YYEUHICPr yyeuhicpr(true);
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

    YYUKRPIr yyukrpir(false);
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
    ext::shared_ptr<YYUKRPIr> iir(new YYUKRPIr(interp));
    ext::shared_ptr<YYUKRPIr> iirYES(new YYUKRPIr(true));
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


void InflationTest::testYYTermStructure() {
    BOOST_TEST_MESSAGE("Testing year-on-year inflation term structure...");

    using namespace inflation_test;

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

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
    ext::shared_ptr<YYUKRPIr> iir(new YYUKRPIr(interp, hy));
    for (Size i=0; i<LENGTH(fixData); i++) {
        iir->addFixing(rpiSchedule[i], fixData[i]);
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

void InflationTest::testPeriod() {
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

void InflationTest::testCpiFlatInterpolation() {
    BOOST_TEST_MESSAGE("Testing CPI flat interpolation...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    Settings::instance().evaluationDate() = Date(10, February, 2022);

    QL_DEPRECATED_DISABLE_WARNING_III_CONSTRUCTOR

    auto testIndex1 = ext::shared_ptr<UKRPI>(new UKRPI(false));
    auto testIndex2 = ext::shared_ptr<UKRPI>(new UKRPI(true));

    QL_DEPRECATED_ENABLE_WARNING_III_CONSTRUCTOR

    testIndex1->addFixing(Date(1, November, 2020), 293.5);
    testIndex1->addFixing(Date(1, December, 2020), 295.4);
    testIndex1->addFixing(Date(1, January,  2021), 294.6);
    testIndex1->addFixing(Date(1, February, 2021), 296.0);
    testIndex1->addFixing(Date(1, March,    2021), 296.9);

    for (auto testIndex: { testIndex1, testIndex2 }) {

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
}

void InflationTest::testCpiLinearInterpolation() {
    BOOST_TEST_MESSAGE("Testing CPI linear interpolation...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    Settings::instance().evaluationDate() = Date(10, February, 2022);

    QL_DEPRECATED_DISABLE_WARNING_III_CONSTRUCTOR

    auto testIndex1 = ext::shared_ptr<UKRPI>(new UKRPI(false));
    auto testIndex2 = ext::shared_ptr<UKRPI>(new UKRPI(true));

    QL_DEPRECATED_ENABLE_WARNING_III_CONSTRUCTOR

    testIndex1->addFixing(Date(1, November, 2020), 293.5);
    testIndex1->addFixing(Date(1, December, 2020), 295.4);
    testIndex1->addFixing(Date(1, January,  2021), 294.6);
    testIndex1->addFixing(Date(1, February, 2021), 296.0);
    testIndex1->addFixing(Date(1, March,    2021), 296.9);

    for (auto testIndex : { testIndex1, testIndex2 }) {
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
}

void InflationTest::testCpiAsIndexInterpolation() {
    BOOST_TEST_MESSAGE("Testing CPI as-index interpolation...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    Date today = Date(10, February, 2022);
    Settings::instance().evaluationDate() = today;

    // AsIndex requires a term structure, even for fixings in the past
    std::vector<Date> dates = { today - 3*Months, today + 5*Years };
    std::vector<Rate> rates = { 0.02, 0.02 };
    Handle<ZeroInflationTermStructure> mock_curve(
            ext::make_shared<ZeroInflationCurve>(today, TARGET(), Actual360(),
                                                 3 * Months, Monthly, dates, rates));

    QL_DEPRECATED_DISABLE_WARNING_III_CONSTRUCTOR

    auto testIndex1 = ext::shared_ptr<UKRPI>(new UKRPI(false, mock_curve));
    auto testIndex2 = ext::shared_ptr<UKRPI>(new UKRPI(true, mock_curve));

    QL_DEPRECATED_ENABLE_WARNING_III_CONSTRUCTOR

    testIndex1->addFixing(Date(1, November, 2020), 293.5);
    testIndex1->addFixing(Date(1, December, 2020), 295.4);
    testIndex1->addFixing(Date(1, January,  2021), 294.6);
    testIndex1->addFixing(Date(1, February, 2021), 296.0);
    testIndex1->addFixing(Date(1, March,    2021), 296.9);

    Real calculated = CPI::laggedFixing(testIndex1, Date(10, February, 2021), 3 * Months, CPI::AsIndex);
    Real expected = 293.5;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex2, Date(10, February, 2021), 3 * Months, CPI::AsIndex);
    expected = 293.5 * (19/28.0) + 295.4 * (9/28.0);

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex1, Date(12, May, 2021), 3 * Months, CPI::AsIndex);
    expected = 296.0;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex2, Date(12, May, 2021), 3 * Months, CPI::AsIndex);
    expected = 296.0 * (20/31.0) + 296.9 * (11/31.0);

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    calculated = CPI::laggedFixing(testIndex1, Date(25, June, 2021), 3 * Months, CPI::AsIndex);
    expected = 296.9;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);

    // this would require April's fixing
    BOOST_CHECK_THROW(calculated = CPI::laggedFixing(testIndex2, Date(25, June, 2021), 3 * Months, CPI::AsIndex),
                      Error);

    // however, this is a special case
    calculated = CPI::laggedFixing(testIndex2, Date(1, June, 2021), 3 * Months, CPI::AsIndex);
    expected = 296.9;

    BOOST_CHECK_MESSAGE(std::fabs(calculated-expected) < 1e-8,
                        "failed to retrieve inflation fixing" <<
                        "\n    expected:   " << expected <<
                        "\n    calculated: " << calculated);
}

test_suite* InflationTest::suite() {

    auto* suite = BOOST_TEST_SUITE("Inflation tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testPeriod));

    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testZeroIndex));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testZeroTermStructure));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testZeroIndexFutureFixing));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testInterpolatedZeroTermStructure));

    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testYYIndex));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testYYTermStructure));

    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testCpiFlatInterpolation));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testCpiLinearInterpolation));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testCpiAsIndexInterpolation));

    return suite;
}
