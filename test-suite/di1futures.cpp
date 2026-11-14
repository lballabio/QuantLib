/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/indexes/ibor/cdi.hpp>
#include <ql/instruments/di1future.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/di1futureratehelper.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <cmath>
#include <iterator>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(Di1FuturesTests)

BOOST_AUTO_TEST_CASE(testUnitPriceOnFlatCurve) {
    BOOST_TEST_MESSAGE("Testing DI1 future unit price on a flat curve...");

    Date today(5, January, 2026);
    Settings::instance().evaluationDate() = today;

    Rate flatRate = 0.1425;
    Handle<YieldTermStructure> curve(
        ext::make_shared<FlatForward>(today, flatRate, Business252(), Compounded, Annual));
    auto convexity = ext::make_shared<SimpleQuote>(0.0);
    Di1Future future(ext::make_shared<Cdi>(curve), Date(1, April, 2026), Handle<Quote>(convexity));

    // business days from January 5th (included) to April 1st (excluded),
    // net of Carnival on February 16th and 17th
    Date::serial_type n = 60;
    BOOST_CHECK_EQUAL(future.businessDays(), n);
    QL_CHECK_SMALL(future.rate() - flatRate, 1e-12);
    QL_CHECK_CLOSE(future.NPV(), 100000.0 / std::pow(1.0 + flatRate, n / 252.0), 1e-10);
    QL_CHECK_CLOSE(future.NPV(), 96877.89912313, 1e-10);
    QL_CHECK_CLOSE(future.unitPrice(), future.NPV(), 1e-12);

    // the convexity adjustment is added to the forward rate
    Real adjustment = 0.001;
    convexity->setValue(adjustment);
    QL_CHECK_SMALL(future.rate() - (flatRate + adjustment), 1e-12);
    QL_CHECK_CLOSE(future.NPV(), 100000.0 / std::pow(1.0 + flatRate + adjustment, n / 252.0),
                   1e-10);

    // the accrual period starts on the evaluation date
    convexity->setValue(0.0);
    Settings::instance().evaluationDate() = Date(6, January, 2026);
    BOOST_CHECK_EQUAL(future.businessDays(), n - 1);
    QL_CHECK_CLOSE(future.NPV(), 100000.0 / std::pow(1.0 + flatRate, (n - 1) / 252.0), 1e-10);
}

BOOST_AUTO_TEST_CASE(testBusinessDaysAroundHolidays) {
    BOOST_TEST_MESSAGE("Testing DI1 future business days around Brazilian holidays...");

    struct {
        Date today;
        Date maturity;
        Date::serial_type businessDays;
    } testCases[] = {
        // Carnival on February 16th and 17th
        {Date(2, February, 2026), Date(2, March, 2026), 18},
        // Republic Day on November 15th and Black Awareness Day on November 20th
        {Date(1, November, 2024), Date(2, December, 2024), 19},
        // All Souls' Day and Republic Day; November 20th was not a national holiday yet
        {Date(1, November, 2023), Date(1, December, 2023), 20},
    };

    auto index = ext::make_shared<Cdi>();
    for (const auto& testCase : testCases) {
        Settings::instance().evaluationDate() = testCase.today;
        Di1Future future(index, testCase.maturity);
        BOOST_CHECK_EQUAL(future.businessDays(), testCase.businessDays);
    }
}

BOOST_AUTO_TEST_CASE(testBootstrap) {
    BOOST_TEST_MESSAGE("Testing bootstrap over DI1 futures...");

    Date today(3, November, 2025);
    Settings::instance().evaluationDate() = today;

    struct {
        Month month;
        Year year;
        Rate rate;
    } quotes[] = {
        {January, 2026, 0.1490}, {February, 2026, 0.1488}, {March, 2026, 0.1480},
        {April, 2026, 0.1470},   {July, 2026, 0.1440},     {January, 2027, 0.1395},
        {January, 2028, 0.1325}, {January, 2029, 0.1320},
    };

    auto index = ext::make_shared<Cdi>();
    std::vector<ext::shared_ptr<RateHelper>> helpers;
    for (const auto& q : quotes)
        helpers.push_back(ext::make_shared<Di1FutureRateHelper>(q.rate, q.month, q.year, index));

    auto curve =
        ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(today, helpers, Business252());
    auto cdi = ext::make_shared<Cdi>(Handle<YieldTermStructure>(curve));

    // business days from November 3rd, 2025 (included) to January 2nd, 2026 (excluded)
    Di1Future first(cdi, Date(2, January, 2026));
    BOOST_CHECK_EQUAL(first.businessDays(), 41);

    for (Size i = 0; i < helpers.size(); ++i) {
        Di1Future future(cdi, helpers[i]->maturityDate());
        QL_CHECK_SMALL(future.rate() - quotes[i].rate, 1e-10);
        QL_CHECK_SMALL(helpers[i]->quoteError(), 1e-10);
    }
}

BOOST_AUTO_TEST_CASE(testMaturityFromMonthAndYear) {
    BOOST_TEST_MESSAGE("Testing DI1 future maturities from month and year...");

    Settings::instance().evaluationDate() = Date(15, December, 2025);

    // first trading sessions of each month, as in the B3 expiration calendar
    const Date expected[] = {
        Date(2, January, 2026), Date(2, February, 2026), Date(2, March, 2026),
        Date(1, April, 2026),   Date(4, May, 2026),      Date(1, June, 2026),
        Date(1, July, 2026),    Date(3, August, 2026),   Date(1, September, 2026),
        Date(1, October, 2026), Date(3, November, 2026), Date(1, December, 2026),
        Date(4, January, 2027), Date(1, February, 2027), Date(1, March, 2027),
        Date(1, April, 2027),   Date(3, May, 2027),      Date(1, June, 2027),
        Date(1, July, 2027),    Date(2, August, 2027),   Date(1, September, 2027),
        Date(1, October, 2027), Date(1, November, 2027), Date(1, December, 2027),
    };

    auto index = ext::make_shared<Cdi>();
    for (Size i = 0; i < std::size(expected); ++i) {
        Month month = Month(i % 12 + 1);
        Year year = Year(2026 + i / 12);
        Di1FutureRateHelper helper(0.14, month, year, index);
        BOOST_CHECK_EQUAL(helper.maturityDate(), expected[i]);
        BOOST_CHECK_EQUAL(helper.future()->maturityDate(), expected[i]);
        BOOST_CHECK_EQUAL(helper.pillarDate(), expected[i]);
    }

    // the earliest date follows the evaluation date, the pillar doesn't
    Di1FutureRateHelper custom(0.14, Date(1, April, 2026), index, 0.0, Pillar::CustomDate,
                               Date(2, February, 2026));
    BOOST_CHECK_EQUAL(custom.earliestDate(), Date(15, December, 2025));
    Settings::instance().evaluationDate() = Date(3, March, 2026);
    BOOST_CHECK_EQUAL(custom.earliestDate(), Date(3, March, 2026));
    BOOST_CHECK_EQUAL(custom.pillarDate(), Date(2, February, 2026));
}

BOOST_AUTO_TEST_CASE(testImpliedQuoteSensitivities) {
    BOOST_TEST_MESSAGE("Testing DI1 future implied quote sensitivities...");

    Date today(3, November, 2025);
    Settings::instance().evaluationDate() = today;

    Date maturity(1, April, 2026);
    auto makeCurve = [&](DiscountFactor maturityDiscount) {
        std::vector<Date> dates = {today, Date(2, January, 2026), maturity, Date(1, July, 2026)};
        std::vector<DiscountFactor> discounts = {1.0, 0.978, maturityDiscount, 0.917};
        return ext::make_shared<DiscountCurve>(dates, discounts, Business252());
    };

    Di1FutureRateHelper helper(0.14, maturity, ext::make_shared<Cdi>(), 0.0005);

    DiscountFactor maturityDiscount = 0.945;
    auto curve = makeCurve(maturityDiscount);
    helper.setTermStructure(curve.get());
    ImpliedQuoteSensitivities sensitivities = helper.impliedQuoteSensitivitiesByCurve();
    BOOST_REQUIRE(sensitivities.available);
    const auto& entries = sensitivities.sensitivities[curve.get()];
    BOOST_REQUIRE_EQUAL(entries.size(), 2U);
    BOOST_CHECK_EQUAL(entries[0].first, today);
    BOOST_CHECK_EQUAL(entries[1].first, maturity);

    Real h = 1.0e-6;
    auto bumpedUp = makeCurve(maturityDiscount + h);
    helper.setTermStructure(bumpedUp.get());
    Real up = helper.impliedQuote();
    auto bumpedDown = makeCurve(maturityDiscount - h);
    helper.setTermStructure(bumpedDown.get());
    Real down = helper.impliedQuote();
    QL_CHECK_CLOSE(entries[1].second, (up - down) / (2.0 * h), 1e-6);

    // the quote depends only on the ratio of the two discount factors,
    // so that P(t0) dQ/dP(t0) + P(T) dQ/dP(T) = 0 with P(t0) = 1
    QL_CHECK_SMALL(entries[0].second + entries[1].second * maturityDiscount, 1e-10);
}

BOOST_AUTO_TEST_CASE(testErrors) {
    BOOST_TEST_MESSAGE("Testing DI1 future error conditions...");

    Date today(3, November, 2025);
    Settings::instance().evaluationDate() = today;
    auto index = ext::make_shared<Cdi>();

    // maturity not later than the evaluation date
    BOOST_CHECK_THROW(Di1FutureRateHelper(0.14, Date(1, October, 2025), index), Error);
    BOOST_CHECK_THROW(Di1FutureRateHelper(0.14, today, index), Error);
    BOOST_CHECK_THROW(Di1FutureRateHelper(0.14, October, 2025, index), Error);

    // invalid custom pillar dates
    Date maturity(2, January, 2026);
    BOOST_CHECK_THROW(Di1FutureRateHelper(0.14, maturity, index, 0.0, Pillar::CustomDate), Error);
    BOOST_CHECK_THROW(Di1FutureRateHelper(0.14, maturity, index, 0.0, Pillar::CustomDate, today),
                      Error);
    BOOST_CHECK_THROW(
        Di1FutureRateHelper(0.14, maturity, index, 0.0, Pillar::CustomDate, Date(5, January, 2026)),
        Error);

    // null index
    BOOST_CHECK_THROW(Di1Future(ext::shared_ptr<OvernightIndex>(), Date(2, January, 2026)), Error);

    // index without a forwarding curve
    Di1Future noCurve(index, Date(2, January, 2026));
    BOOST_CHECK_THROW(noCurve.NPV(), Error);
    BOOST_CHECK_THROW(noCurve.rate(), Error);

    // no business days left: evaluation on Saturday, maturity on Monday
    Settings::instance().evaluationDate() = Date(31, January, 2026);
    Handle<YieldTermStructure> curve(ext::make_shared<FlatForward>(
        Date(31, January, 2026), 0.14, Business252(), Compounded, Annual));
    Di1Future noDays(ext::make_shared<Cdi>(curve), Date(2, February, 2026));
    BOOST_CHECK_EQUAL(noDays.businessDays(), 0);
    BOOST_CHECK_THROW(noDays.NPV(), Error);

    // the contract expires on its maturity date
    Settings::instance().evaluationDate() = Date(2, February, 2026);
    BOOST_CHECK(noDays.isExpired());
    QL_CHECK_SMALL(noDays.NPV(), 1e-12);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
