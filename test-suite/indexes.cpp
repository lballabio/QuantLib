/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 StatPro Italia srl

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

#include "indexes.hpp"
#include "utilities.hpp"
#include <ql/indexes/bmaindex.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void IndexTest::testFixingObservability() {
    BOOST_TEST_MESSAGE("Testing observability of index fixings...");

    ext::shared_ptr<InterestRateIndex> i1 = ext::make_shared<Euribor6M>();
    ext::shared_ptr<InterestRateIndex> i2 = ext::make_shared<BMAIndex>();

    Flag f1;
    f1.registerWith(i1);
    f1.lower();

    Flag f2;
    f2.registerWith(i2);
    f2.lower();

    Date today = Date::todaysDate();

    ext::shared_ptr<Index> euribor = ext::make_shared<Euribor6M>();

    Date d1 = today;
    while (!euribor->isValidFixingDate(d1))
        d1++;

    euribor->addFixing(d1, -0.003);
    if (!f1.isUp())
        BOOST_FAIL("Observer was not notified of added Euribor fixing");

    ext::shared_ptr<Index> bma = ext::make_shared<BMAIndex>();

    Date d2 = today;
    while (!bma->isValidFixingDate(d2))
        d2++;

    bma->addFixing(d2, 0.01);
    if (!f2.isUp())
        BOOST_FAIL("Observer was not notified of added BMA fixing");
}

void IndexTest::testFixingHasHistoricalFixing() {
    BOOST_TEST_MESSAGE("Testing if index has historical fixings...");

    auto testCase = [](const std::string& indexName, const bool& expected, const bool& testResult) {
        if (expected != testResult) {
            BOOST_FAIL("Historical fixing " << (testResult ? "" : "not ") << "found for "
                                            << indexName << ".");
        }
    };

    std::string name;
    auto fixingFound = true;
    auto fixingNotFound = false;

    Date today = Date::todaysDate();

    IndexManager::instance().clearHistories();

    auto euribor3M = ext::make_shared<Euribor3M>();
    auto euribor6M = ext::make_shared<Euribor6M>();
    auto euribor6M_a = ext::make_shared<Euribor6M>();
    euribor6M->addFixing(today, 0.01);

    name = euribor3M->name();
    testCase(name, fixingNotFound, euribor3M->hasHistoricalFixing(today));
    testCase(name, fixingNotFound, IndexManager::instance().hasHistoricalFixing(name, today));

    name = euribor6M->name();
    testCase(name, fixingFound, euribor6M->hasHistoricalFixing(today));
    testCase(name, fixingFound, euribor6M_a->hasHistoricalFixing(today));
    testCase(name, fixingFound, IndexManager::instance().hasHistoricalFixing(name, today));

    IndexManager::instance().clearHistories();

    name = euribor3M->name();
    testCase(name, fixingNotFound, euribor3M->hasHistoricalFixing(today));
    testCase(name, fixingNotFound, IndexManager::instance().hasHistoricalFixing(name, today));

    name = euribor6M->name();
    testCase(name, fixingNotFound, euribor6M->hasHistoricalFixing(today));
    testCase(name, fixingNotFound, euribor6M_a->hasHistoricalFixing(today));
    testCase(name, fixingNotFound, IndexManager::instance().hasHistoricalFixing(name, today));
}


test_suite* IndexTest::suite() {
    auto* suite = BOOST_TEST_SUITE("index tests");
    suite->add(QUANTLIB_TEST_CASE(&IndexTest::testFixingObservability));
    suite->add(QUANTLIB_TEST_CASE(&IndexTest::testFixingHasHistoricalFixing));
    return suite;
}
