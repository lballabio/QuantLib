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

using ZeroCouponInflationSwapHelpers =
    std::vector<ext::shared_ptr<BootstrapHelper<ZeroInflationTermStructure> > >;

namespace ZCIIS {
    struct Result {
        ext::shared_ptr<ZeroInflationIndex> index;
        ZeroCouponInflationSwapHelpers helpers;
        ext::shared_ptr<ZeroInflationCurve> curve;
        ext::shared_ptr<ZeroCouponInflationSwap> swap;
    };

    struct Setup {
        Setup(bool indexIsInterpolated, CPI::InterpolationType observationInterpolation)
        : indexIsInterpolated(indexIsInterpolated),
          observationInterpolation(observationInterpolation) {
            evaluationDate = calendar.adjust(unadjustedEvaluationDate);
            Settings::instance().evaluationDate() = evaluationDate;

            nominalTermStructure = ext::shared_ptr<YieldTermStructure>(
                new FlatForward(evaluationDate, 0.05, Actual360()));
        }

        bool indexIsInterpolated;
        CPI::InterpolationType observationInterpolation;

        Date unadjustedEvaluationDate{Date(13, August, 2007)};
        Date evaluationDate;

        Calendar calendar{UnitedKingdom()};
        BusinessDayConvention bdc{ModifiedFollowing};
        DayCounter dc{Thirty360(Thirty360::BondBasis)};
        Period observationLag{Period(3, Months)};

        ZeroCouponInflationSwap::Type swapType{ZeroCouponInflationSwap::Payer};
        Real swapNomimal{1000000.00};

        Date indexFixingScheduleFrom{Date(1, January, 2005)};
        Date indexFixingScheduleTo{Date(13, August, 2007)};

        ext::shared_ptr<YieldTermStructure> nominalTermStructure;
    };

    std::ostream& operator<<(std::ostream& out, const Setup& setup) {
        std::string observationInterpolation;
        switch (setup.observationInterpolation) {
            case CPI::AsIndex:
                observationInterpolation = "CPI::AsIndex";
                break;
            case CPI::Flat:
                observationInterpolation = "CPI::Flat";
                break;
            case CPI::Linear:
                observationInterpolation = "CPI::Linear";
                break;
            default:
                QL_FAIL("Unknown CPI::InterpolationType...");
        }

        return out << observationInterpolation << " (Index "
                   << (setup.indexIsInterpolated ? "" : "not ") << "interpolated)";
    }

    Real fixData[] = {189.9, 189.9, 189.6, 190.5, 191.6, 192.0, 192.2, 192.2, 192.6, 193.1, 193.3,
                      193.6, 194.1, 193.4, 194.2, 195.0, 196.5, 197.7, 198.5, 198.5, 199.2, 200.1,
                      200.4, 201.1, 202.7, 201.6, 203.1, 204.4, 205.4, 206.2, 207.3};

    ext::shared_ptr<UKRPI> makeUKRPI(const Setup& setup,
                                     const RelinkableHandle<ZeroInflationTermStructure>& hz) {
        // fixing data
        Schedule rpiSchedule = MakeSchedule()
                                   .from(setup.indexFixingScheduleFrom)
                                   .to(setup.indexFixingScheduleTo)
                                   .withTenor(1 * Months)
                                   .withCalendar(setup.calendar)
                                   .withConvention(setup.bdc);

        ext::shared_ptr<UKRPI> ukrpi(new UKRPI(setup.indexIsInterpolated, hz));
        for (Size i = 0; i < LENGTH(fixData); i++) {
            ukrpi->addFixing(rpiSchedule[i], fixData[i]);
        }

        return ukrpi;
    }

    struct Datum {
        Date date;
        Rate rate;
    };

    std::vector<Datum> zcData = {{Date(13, August, 2008), 2.93},  {Date(13, August, 2009), 2.95},
                                 {Date(13, August, 2010), 2.965}, {Date(15, August, 2011), 2.98},
                                 {Date(13, August, 2012), 3.0},   {Date(13, August, 2014), 3.06},
                                 {Date(13, August, 2017), 3.175}, {Date(13, August, 2019), 3.243},
                                 {Date(15, August, 2022), 3.293}, {Date(14, August, 2027), 3.338},
                                 {Date(13, August, 2032), 3.348}, {Date(15, August, 2037), 3.348},
                                 {Date(13, August, 2047), 3.308}, {Date(13, August, 2057), 3.228}};


    ZeroCouponInflationSwapHelpers makeHelpers(const Setup& setup,
                                               const ext::shared_ptr<ZeroInflationIndex>& index) {
        auto makeHelper = [&setup, &index](const Handle<Quote>& quote, const Date& maturity) {
            return ext::make_shared<ZeroCouponInflationSwapHelper>(
                quote, setup.observationLag, maturity, setup.calendar, setup.bdc, setup.dc, index,
                setup.observationInterpolation,
                Handle<YieldTermStructure>(setup.nominalTermStructure));
        };

        ZeroCouponInflationSwapHelpers instruments;
        for (Datum datum : zcData) {
            Date maturity = datum.date;
            Handle<Quote> quote(ext::shared_ptr<Quote>(new SimpleQuote(datum.rate / 100.0)));
            auto anInstrument = makeHelper(quote, maturity);
            instruments.push_back(anInstrument);
        }

        return instruments;
    }

    ext::shared_ptr<ZeroInflationCurve>
    makeZeroInflationCurve(const Setup& setup,
                           const ext::shared_ptr<ZeroInflationIndex>& index,
                           const ZeroCouponInflationSwapHelpers& helpers) {
        auto frequency = Monthly;

        Rate baseZeroRate = zcData[0].rate / 100.0;
        ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITS(
            new PiecewiseZeroInflationCurve<Linear>(setup.evaluationDate, setup.calendar, setup.dc,
                                                    setup.observationLag, frequency,
                                                    baseZeroRate, helpers));
        pZITS->recalculate();

        return pZITS;
    }

    ext::shared_ptr<ZeroCouponInflationSwap>
    makeZeroCouponInflationSwap(const Setup& setup,
                                const ext::shared_ptr<ZeroInflationIndex>& index,
                                const ext::shared_ptr<ZeroInflationCurve>& curve) {
        auto maturity = zcData[6].date;
        auto fixedRate = zcData[6].rate / 100.0;

        return ext::make_shared<ZeroCouponInflationSwap>(
            setup.swapType, setup.swapNomimal, setup.evaluationDate, maturity, setup.calendar,
            setup.bdc, setup.dc, fixedRate, index, setup.observationLag,
            setup.observationInterpolation);
    }

    Result makeResult(const Setup& setup) {
        Result result;

        RelinkableHandle<ZeroInflationTermStructure> hz;

        result.index = makeUKRPI(setup, hz);
        result.helpers = makeHelpers(setup, result.index);
        result.curve = makeZeroInflationCurve(setup, result.index, result.helpers);
        result.swap = makeZeroCouponInflationSwap(setup, result.index, result.curve);

        hz.linkTo(result.curve);

        return result;
    }

    void runTest(const Setup& setup) {
        BOOST_TEST_MESSAGE("Testing ZCIIS " << setup << "...");

        auto result = ZCIIS::makeResult(setup);
    }
}

void InflationZCIISInterpolationTest::asIndexNotInterpolated() {
    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    bool indexIsInterpolated = false;
    const auto setup = ZCIIS::Setup(indexIsInterpolated, CPI::AsIndex);

    ZCIIS::runTest(setup);
}

void InflationZCIISInterpolationTest::asIndexInterpolated() {
    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    bool indexIsInterpolated = true;
    const auto setup = ZCIIS::Setup(indexIsInterpolated, CPI::AsIndex);

    ZCIIS::runTest(setup);
}

void InflationZCIISInterpolationTest::flatNotInterpolated() {
    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    bool indexIsInterpolated = false;
    const auto setup = ZCIIS::Setup(indexIsInterpolated, CPI::Flat);

    ZCIIS::runTest(setup);
}

void InflationZCIISInterpolationTest::flatInterpolated() {
    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    bool indexIsInterpolated = true;
    const auto setup = ZCIIS::Setup(indexIsInterpolated, CPI::Flat);

    ZCIIS::runTest(setup);
}

void InflationZCIISInterpolationTest::linearNotInterpolated() {
    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    bool indexIsInterpolated = false;
    const auto setup = ZCIIS::Setup(indexIsInterpolated, CPI::Linear);

    ZCIIS::runTest(setup);
}

void InflationZCIISInterpolationTest::linearInterpolated() {
    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    bool indexIsInterpolated = true;
    const auto setup = ZCIIS::Setup(indexIsInterpolated, CPI::Linear);

    ZCIIS::runTest(setup);
}

test_suite* InflationZCIISInterpolationTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Zero Coupon Inflation Index Swap (ZCIIS) interpolation tests");

    //suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::asIndexNotInterpolated));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::asIndexInterpolated));

    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::flatNotInterpolated));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::flatInterpolated));

    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::linearNotInterpolated));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::linearInterpolated));

    return suite;
}
