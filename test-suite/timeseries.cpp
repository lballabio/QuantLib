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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "timeseries.hpp"
#include "utilities.hpp"
#include <ql/timeseries.hpp>
#include <ql/prices.hpp>
#include <ql/time/calendars/unitedstates.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/unordered_map.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

void TimeSeriesTest::testConstruction() {

    BOOST_TEST_MESSAGE("Testing time series construction...");

    TimeSeries<Real> ts;
    ts[Date(25, March, 2005)] = 1.2;
    ts[Date(29, March, 2005)] = 2.3;
    ts[Date(15, March, 2005)] = 0.3;

    TimeSeries<Real>::const_iterator cur = ts.begin();
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

void TimeSeriesTest::testIntervalPrice() {
    BOOST_TEST_MESSAGE("Testing time series interval price...");

    std::vector<Date> date;
    std::vector<Real> open, close, high, low;
    date.push_back(Date(25, March, 2005));
    date.push_back(Date(29, March, 2005));

    open.push_back(1.3);
    open.push_back(2.3);

    close.push_back(2.3);
    close.push_back(3.4);

    high.push_back(3.4);
    high.push_back(3.5);

    low.push_back(3.4);
    low.push_back(3.2);

    TimeSeries<IntervalPrice> tsiq = IntervalPrice::makeSeries(date,
                                                               open,
                                                               close,
                                                               high,
                                                               low);
}

void TimeSeriesTest::testIterators() {
    BOOST_TEST_MESSAGE("Testing time series iterators...");
    std::vector<Date> dates;
    std::vector<Real> prices;

    dates.push_back(Date(25, March, 2005));
    dates.push_back(Date(29, March, 2005));
    dates.push_back(Date(15, March, 2005));

    prices.push_back(25);
    prices.push_back(23);
    prices.push_back(20);

    TimeSeries<Real> ts(dates.begin(), dates.end(), prices.begin());

    // projection iterators

    std::copy(ts.cbegin_time(), ts.cend_time(), dates.begin());
    if (dates[0] != Date(15, March, 2005)) {
        BOOST_ERROR("date does not match");
    }

    std::copy(ts.cbegin_values(), ts.cend_values(), prices.begin());
    if (prices[0] != 20) {
        BOOST_ERROR("value does not match");
    }

    dates = ts.dates(); 
    if (dates[0] != Date(15, March, 2005)) {
        BOOST_ERROR("date does not match");
    }

    prices = ts.values();
    if (prices[0] != 20) {
        BOOST_ERROR("value does not match");
    }

    // unordered container
    typedef TimeSeries<int, boost::unordered_map<Date, int> >
                                                          TimeSeriesUnordered;
    TimeSeriesUnordered ts1;
    Date d0(25, March, 2005), d1(25, April, 2005), d = d0;
    UnitedStates calendar(UnitedStates::NYSE);
    for (int i = 0; d < d1; ++i, d = calendar.advance(d, 1, Days)) {
        ts1[d] = i;
    }

    d = d0;
    for (int i = 0; d < d1; ++i, d = calendar.advance(d, 1, Days)) {
        if (ts1[d] != int(i)) {
            BOOST_ERROR("value does not match");
        }
    }

    // reverse iterators

    std::vector<std::pair<Date,Real> > data(prices.size());
    std::copy(ts.crbegin(), ts.crend(), data.begin());
    if (data[2].second != 20) {
        BOOST_ERROR("value does not match");
    }
    if (data[2].first != Date(15, March, 2005)) {
        BOOST_ERROR("date does not match");
    }

    std::copy(ts.crbegin_time(), ts.crend_time(), dates.begin());
    if (dates[0] != Date(29, March, 2005)) {
        BOOST_ERROR("date does not match");
    }

    std::copy(ts.crbegin_values(), ts.crend_values(), prices.begin());
    if (prices[0] != 23) {
        BOOST_ERROR("value does not match");
    }

    // The following should not compile:
    // std::transform(ts1.crbegin(), ts1.crend(), prices.begin(),
    //                TimeSeriesUnordered::get_value);
    // std::copy(ts1.crbegin_values(), ts1.crend_values(), prices.begin());
    // ts1.lastDate();

    // last date 
    if (ts.lastDate() != Date(29, March, 2005)) {
        BOOST_ERROR("lastDate does not match");
    }
}

test_suite* TimeSeriesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("time series tests");
    suite->add(QUANTLIB_TEST_CASE(&TimeSeriesTest::testConstruction));
    suite->add(QUANTLIB_TEST_CASE(&TimeSeriesTest::testIntervalPrice));
    suite->add(QUANTLIB_TEST_CASE(&TimeSeriesTest::testIterators));
    return suite;
}

