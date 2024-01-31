/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Paul Xi Cao

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
#include <ql/prices.hpp>
#include <ql/time/date.hpp>
#include <array>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(PriceTests)

BOOST_AUTO_TEST_CASE(testMidEquivalent) {
    BOOST_TEST_MESSAGE("Testing midEquivalent()...");

    using boost::test_tools::tolerance;

    BOOST_TEST(1.5 == midEquivalent(1, 2, 3, 4), tolerance(1e-14));
    BOOST_TEST(1.5 == midEquivalent(1, 2, 0, 4), tolerance(1e-14));
    BOOST_TEST(1.5 == midEquivalent(1, 2, 3, 0), tolerance(1e-14));
    BOOST_TEST(1.5 == midEquivalent(1, 2, 0, 0), tolerance(1e-14));

    BOOST_TEST(1 == midEquivalent(1, 0, 3, 4));
    BOOST_TEST(1 == midEquivalent(1, 0, 0, 4));
    BOOST_TEST(1 == midEquivalent(1, 0, 3, 0));
    BOOST_TEST(1 == midEquivalent(1, 0, 0, 0));

    BOOST_TEST(2 == midEquivalent(0, 2, 3, 4));
    BOOST_TEST(2 == midEquivalent(0, 2, 0, 4));
    BOOST_TEST(2 == midEquivalent(0, 2, 3, 0));
    BOOST_TEST(2 == midEquivalent(0, 2, 0, 0));

    BOOST_TEST(3 == midEquivalent(0, 0, 3, 4));
    BOOST_TEST(4 == midEquivalent(0, 0, 0, 4));
    BOOST_TEST(3 == midEquivalent(0, 0, 3, 0));

    BOOST_CHECK_THROW(midEquivalent(0, 0, 0, 0), QuantLib::Error);
}

BOOST_AUTO_TEST_CASE(testMidSafe) {
    BOOST_TEST_MESSAGE("Testing midSafe()...");

    using boost::test_tools::tolerance;

    BOOST_TEST(1.5 == midSafe(1, 2), tolerance(1e-14));

    BOOST_CHECK_THROW(midSafe(0, 0), QuantLib::Error);
    BOOST_CHECK_THROW(midSafe(1, 0), QuantLib::Error);
    BOOST_CHECK_THROW(midSafe(0, 2), QuantLib::Error);
}

BOOST_AUTO_TEST_CASE(testIntervalPriceInspectors) {
    BOOST_TEST_MESSAGE("Testing IntervalPrice inspectors...");

    const IntervalPrice p(1, 2, 3, 4);

    BOOST_TEST(1 == p.open());
    BOOST_TEST(1 == p.value(IntervalPrice::Open));

    BOOST_TEST(2 == p.close());
    BOOST_TEST(2 == p.value(IntervalPrice::Close));

    BOOST_TEST(3 == p.high());
    BOOST_TEST(3 == p.value(IntervalPrice::High));

    BOOST_TEST(4 == p.low());
    BOOST_TEST(4 == p.value(IntervalPrice::Low));
}

void testEquality(const IntervalPrice& lhs, const IntervalPrice& rhs) {
    using T = IntervalPrice::Type;
    for (const auto t : {T::Open, T::Close, T::High, T::Low})
        BOOST_TEST(lhs.value(t) == rhs.value(t));
}

BOOST_AUTO_TEST_CASE(testIntervalPriceModifiers) {
    BOOST_TEST_MESSAGE("Testing IntervalPrice modifiers...");

    IntervalPrice p(1, 2, 3, 4);

    p.setValue(11, IntervalPrice::Open);
    testEquality(p, IntervalPrice(11, 2, 3, 4));

    p.setValue(12, IntervalPrice::Close);
    testEquality(p, IntervalPrice(11, 12, 3, 4));

    p.setValue(13, IntervalPrice::High);
    testEquality(p, IntervalPrice(11, 12, 13, 4));

    p.setValue(14, IntervalPrice::Low);
    testEquality(p, IntervalPrice(11, 12, 13, 14));

    p.setValues(21, 22, 23, 24);
    testEquality(p, IntervalPrice(21, 22, 23, 24));
}

TimeSeries<IntervalPrice> createSeries() {
    std::vector<Date> d;
    d.emplace_back((Day)1, (Month)1, (Year)2001);
    d.emplace_back((Day)3, (Month)3, (Year)2003);
    d.emplace_back((Day)2, (Month)2, (Year)2002);

    const std::vector<Real> open{11, 13, 12};
    const std::vector<Real> close{21, 23, 22};
    const std::vector<Real> high{31, 33, 32};
    const std::vector<Real> low{41, 43, 42};

    return IntervalPrice::makeSeries(d, open, close, high, low);
}

BOOST_AUTO_TEST_CASE(testIntervalPriceMakeSeries) {
    BOOST_TEST_MESSAGE("Testing creation of IntervalPrice series...");

    const TimeSeries<IntervalPrice> priceSeries = createSeries();

    BOOST_TEST(3U == priceSeries.size());
    testEquality(priceSeries[{(Day)1, (Month)1, (Year)2001}], {11, 21, 31, 41});
    testEquality(priceSeries[{(Day)2, (Month)2, (Year)2002}], {12, 22, 32, 42});
    testEquality(priceSeries[{(Day)3, (Month)3, (Year)2003}], {13, 23, 33, 43});
}

BOOST_AUTO_TEST_CASE(testIntervalPriceExtractComponent) {
    BOOST_TEST_MESSAGE("Testing extraction of IntervalPrice values...");

    const TimeSeries<Real> openSeries =
        IntervalPrice::extractComponent(createSeries(), IntervalPrice::Open);
    const TimeSeries<Real> closeSeries =
        IntervalPrice::extractComponent(createSeries(), IntervalPrice::Close);
    const TimeSeries<Real> highSeries =
        IntervalPrice::extractComponent(createSeries(), IntervalPrice::High);
    const TimeSeries<Real> lowSeries =
        IntervalPrice::extractComponent(createSeries(), IntervalPrice::Low);

    for (const auto& series : {openSeries, closeSeries, highSeries, lowSeries})
        BOOST_TEST(3U == series.size());

    const std::array<Date, 3> expectedDates{Date{(Day)1, (Month)1, (Year)2001},
                                            Date{(Day)2, (Month)2, (Year)2002},
                                            Date{(Day)3, (Month)3, (Year)2003}};
    auto expectedDate = expectedDates.begin();  // NOLINT(readability-qualified-auto)

    const std::array<IntervalPrice, 3> expectedPrices{IntervalPrice{11, 21, 31, 41},
                                                      IntervalPrice{12, 22, 32, 42},
                                                      IntervalPrice{13, 23, 33, 43}};
    auto expectedPrice = expectedPrices.begin();  // NOLINT(readability-qualified-auto)

    for (auto openIt = openSeries.begin(), closeIt = closeSeries.begin(),
             highIt = highSeries.begin(), lowIt = lowSeries.begin();
         openIt != openSeries.end();
         ++openIt, ++closeIt, ++highIt, ++lowIt, ++expectedDate, ++expectedPrice) {
        const Date openDate = openIt->first;
        const Real openValue = openIt->second;
        const Date closeDate = closeIt->first;
        const Real closeValue = closeIt->second;
        const Date highDate = highIt->first;
        const Real highValue = highIt->second;
        const Date lowDate = lowIt->first;
        const Real lowValue = lowIt->second;

        for (const Date& date : {openDate, closeDate, highDate, lowDate})
            BOOST_TEST(date == *expectedDate);

        testEquality(*expectedPrice, IntervalPrice(openValue, closeValue, highValue, lowValue));
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
