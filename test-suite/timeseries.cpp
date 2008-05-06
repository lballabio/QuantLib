/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

using namespace QuantLib;
using namespace boost::unit_test_framework;

void TimeSeriesTest::testConstruction() {

    BOOST_MESSAGE("Testing time series construction...");

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
    BOOST_MESSAGE("Testing time series interval price...");

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

test_suite* TimeSeriesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("time series tests");
    suite->add(QUANTLIB_TEST_CASE(&TimeSeriesTest::testConstruction));
    suite->add(QUANTLIB_TEST_CASE(&TimeSeriesTest::testIntervalPrice));
    return suite;
}

