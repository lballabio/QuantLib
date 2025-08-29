/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2010 Liquidnet Holdings, Inc.

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
#include <ql/timeseries.hpp>
#include <ql/prices.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <boost/unordered_map.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TimeSeriesTests)

BOOST_AUTO_TEST_CASE(testConstruction) {

    BOOST_TEST_MESSAGE("Testing time series construction...");

    TimeSeries<Real> ts;
    ts[Date(25, March, 2005)] = 1.2;
    ts[Date(29, March, 2005)] = 2.3;
    ts[Date(15, March, 2005)] = 0.3;

    auto cur = ts.begin();
    if (cur->first != Date(15, March, 2005)) {
        BOOST_ERROR("date does not match");
    }
    if (cur->second != 0.3) {
        BOOST_ERROR("value does not match");
    }

    ts[Date(15, March, 2005)] = 4.0;
    cur = ts.begin();
    if (cur->second != 4.0) {
        BOOST_ERROR("replaced value does not match" << cur->second << "\n");
    }

    ts[Date(15, March, 2005)] = 3.5;

    if (cur->second != 3.5) {
        BOOST_ERROR("set value operator not match" << cur->second << "\n");
    }
}

BOOST_AUTO_TEST_CASE(testIntervalPrice) {
    BOOST_TEST_MESSAGE("Testing time series interval price...");

    std::vector<Date> date = {Date(25, March, 2005), Date(29, March, 2005)};

    std::vector<Real> open = {1.3, 2.3},
                      close = {2.3, 3.4},
                      high = {3.4, 3.5},
                      low = {3.4, 3.2};

    TimeSeries<IntervalPrice> tsiq = IntervalPrice::makeSeries(date,
                                                               open,
                                                               close,
                                                               high,
                                                               low);
}

BOOST_AUTO_TEST_CASE(testIteratingDefaultContainer) {
    BOOST_TEST_MESSAGE("Testing iteration of time series with a default container which sorts by date...");

    const std::vector<Date> dates = {Date(25, March, 2005),
                                     Date(29, March, 2005),
                                     Date(15, March, 2005)};

    const std::vector<Real> prices = {25, 23, 20};

    const TimeSeries<Real> ts(dates.begin(), dates.end(), prices.begin());

    // accessing dates
    {
        std::vector<Date> tsDates;
        std::transform(ts.begin(), ts.end(), std::back_inserter(tsDates),
            [](const std::pair<const Date, Real>& x) -> Date { return x.first; });
        const std::vector<Date> expected{dates[2], dates[0], dates[1]};
        BOOST_TEST(tsDates == expected);
    }

    // accessing Values
    {
        std::vector<Real> tsValues;
        std::transform(ts.begin(), ts.end(), std::back_inserter(tsValues),
            [](const std::pair<const Date, Real>& x) -> Real { return x.second; });
        const std::vector<Real> expected{prices[2], prices[0], prices[1]};
        BOOST_TEST(tsValues == expected);
    }
}

BOOST_AUTO_TEST_CASE(testCustomContainer) {
    BOOST_TEST_MESSAGE("Testing usage of a custom container for time series data...");

    // unordered container
    typedef TimeSeries<int, boost::unordered_map<Date, int> >
                                                          TimeSeriesUnordered;
    TimeSeriesUnordered ts;
    Date d0(25, March, 2005), d1(25, April, 2005), d = d0;
    UnitedStates calendar(UnitedStates::NYSE);
    for (int i = 0; d < d1; ++i, d = calendar.advance(d, 1, Days)) {
        ts[d] = i;
    }

    d = d0;
    for (int i = 0; d < d1; ++i, d = calendar.advance(d, 1, Days)) {
        BOOST_TEST(ts[d] == i);
    }
}

BOOST_AUTO_TEST_CASE(testInspectors) {
    BOOST_TEST_MESSAGE("Testing inspectors of time series...");

    const std::vector<Date> dates = {Date(25, March, 2005),
                                     Date(29, March, 2005),
                                     Date(15, March, 2005)};

    const std::vector<Real> prices = {25, 23, 20};

    const TimeSeries<Real> ts(dates.begin(), dates.end(), prices.begin());

    BOOST_TEST(ts.firstDate() == Date(15, March, 2005));
    BOOST_TEST(ts.lastDate() == Date(29, March, 2005));
    BOOST_TEST(ts.size() == 3);
    BOOST_TEST(!ts.empty());
}

BOOST_AUTO_TEST_CASE(testUtilities) {
    BOOST_TEST_MESSAGE("Testing time series utilities...");

    const std::vector<Date> dates = {Date(25, March, 2005),
                                     Date(29, March, 2005),
                                     Date(15, March, 2005)};

    const std::vector<Real> prices = {25, 23, 20};

    // find: needs mutable TimeSeries object as it might insert
    {
        TimeSeries<Real> ts(dates.begin(), dates.end(), prices.begin());

        BOOST_TEST(ts.find(Date(15, March, 2005))->first == Date(15, March, 2005));
        BOOST_TEST(ts.find(Date(15, March, 2005))->second == 20);
        BOOST_TEST(3 == ts.size());

        BOOST_TEST(ts.find(Date(25, March, 2005))->first == Date(25, March, 2005));
        BOOST_TEST(ts.find(Date(25, March, 2005))->second == 25);
        BOOST_TEST(3 == ts.size());

        BOOST_TEST(ts.find(Date(29, March, 2005))->first == Date(29, March, 2005));
        BOOST_TEST(ts.find(Date(29, March, 2005))->second == 23);
        BOOST_TEST(3 == ts.size());

        BOOST_TEST(ts.find(Date(1, March, 2005))->first == Date(1, March, 2005));
        BOOST_TEST(4 == ts.size());
    }

    const TimeSeries<Real> ts(dates.begin(), dates.end(), prices.begin());

    // dates()
    {
        const std::vector<Date> expected{dates[2], dates[0], dates[1]};
        BOOST_TEST(ts.dates() == expected);
    }

    // values()
    {
        const std::vector<Real> expected{prices[2], prices[0], prices[1]};
        BOOST_TEST(ts.values() == expected);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
