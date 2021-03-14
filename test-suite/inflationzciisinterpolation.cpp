/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include "inflationzciisinterpolation.hpp"
#include "utilities.hpp"
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <iostream>

using boost::unit_test_framework::test_suite;
using namespace QuantLib;

namespace ZCIIS_test {
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

    ext::shared_ptr<ZeroCouponInflationSwap>
    makeZCIIS(CPI::InterpolationType observationInterpolation) {
        // try the Zero UK
        Calendar calendar = UnitedKingdom();
        BusinessDayConvention bdc = ModifiedFollowing;
        Date evaluationDate(13, August, 2007);
        evaluationDate = calendar.adjust(evaluationDate);
        Settings::instance().evaluationDate() = evaluationDate;

        // fixing data
        Date from(1, January, 2005);
        Date to(13, August, 2007);
        Schedule rpiSchedule = MakeSchedule()
                                   .from(from)
                                   .to(to)
                                   .withTenor(1 * Months)
                                   .withCalendar(UnitedKingdom())
                                   .withConvention(ModifiedFollowing);

        Real fixData[] = {189.9, 189.9, 189.6, 190.5, 191.6, 192.0, 192.2, 192.2,
                          192.6, 193.1, 193.3, 193.6, 194.1, 193.4, 194.2, 195.0,
                          196.5, 197.7, 198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
                          202.7, 201.6, 203.1, 204.4, 205.4, 206.2, 207.3};

        RelinkableHandle<ZeroInflationTermStructure> hz;
        bool interp = false;
        ext::shared_ptr<UKRPI> iiUKRPI(new UKRPI(interp, hz));
        for (Size i = 0; i < LENGTH(fixData); i++) {
            iiUKRPI->addFixing(rpiSchedule[i], fixData[i]);
        }

        ext::shared_ptr<ZeroInflationIndex> ii =
            ext::dynamic_pointer_cast<ZeroInflationIndex>(iiUKRPI);
        ext::shared_ptr<YieldTermStructure> nominalTS = nominalTermStructure();

        // now build the zero inflation curve
        std::vector<Datum> zcData = {
            {Date(13, August, 2008), 2.93},  {Date(13, August, 2009), 2.95},
            {Date(13, August, 2010), 2.965}, {Date(15, August, 2011), 2.98},
            {Date(13, August, 2012), 3.0},   {Date(13, August, 2014), 3.06},
            {Date(13, August, 2017), 3.175}, {Date(13, August, 2019), 3.243},
            {Date(15, August, 2022), 3.293}, {Date(14, August, 2027), 3.338},
            {Date(13, August, 2032), 3.348}, {Date(15, August, 2037), 3.348},
            {Date(13, August, 2047), 3.308}, {Date(13, August, 2057), 3.228}};

        Period observationLag = Period(2, Months);
        DayCounter dc = Thirty360();
        Frequency frequency = Monthly;

        auto makeHelper = [&](const Handle<Quote>& quote, const Date& maturity) {
            return ext::make_shared<ZeroCouponInflationSwapHelper>(
                quote, observationLag, maturity, calendar, bdc, dc, ii, observationInterpolation,
                Handle<YieldTermStructure>(nominalTS));
        };
        auto helpers = makeHelpers<ZeroInflationTermStructure>(zcData, makeHelper);

        Rate baseZeroRate = zcData[0].rate / 100.0;
        ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITS(
            new PiecewiseZeroInflationCurve<Linear>(evaluationDate, calendar, dc, observationLag,
                                                    frequency, ii->interpolated(), baseZeroRate,
                                                    helpers));
        pZITS->recalculate();


        return ext::shared_ptr<ZeroCouponInflationSwap>();
    }
}

void InflationZCIISInterpolationTest::testZCIISAsIndex() {
    BOOST_TEST_MESSAGE("Testing ZCIIS CPI::AsIndex interpolation type...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;
}

void InflationZCIISInterpolationTest::testZCIISFlat() {
    BOOST_TEST_MESSAGE("Testing ZCIIS CPI::Flat interpolation type...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;
}

void InflationZCIISInterpolationTest::testZCIISLinear() {
    BOOST_TEST_MESSAGE("Testing ZCIIS CPI::Linear interpolation type...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;
}

test_suite* InflationZCIISInterpolationTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Zero Coupon Inflation Index Swap (ZCIIS) interpolation tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::testZCIISAsIndex));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::testZCIISFlat));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::testZCIISLinear));

    return suite;
}
