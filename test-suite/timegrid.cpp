/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
This file is part of QuantLib, a free - software / open - source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software : you can redistribute it and/or modify it
under the terms of the QuantLib license.You should have received a
copy of the license along with this program; if not, please email
<quantlib - dev@lists.sf.net>.The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.See the license for more details.
*/

#include "ql/timegrid.hpp"
#include "timegrid.hpp"
#include "utilities.hpp"

#include <iostream>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void TimeGridTest::testConstructorAdditionalSteps()
{
    BOOST_TEST_MESSAGE("TimeGrid constructor with additional steps.");
    const std::vector<Time> test_times = { 1, 2, 4 };
    const TimeGrid tg(test_times.begin(), test_times.end(), 8);
    
    // Expect 8 evenly sized steps over the interval [0, 4].
    const std::vector<Time> expected_times = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
    	tg.begin(), tg.end(), expected_times.begin(), expected_times.end());
}

void TimeGridTest::testConstructorMandatorySteps()
{
    BOOST_TEST_MESSAGE("TimeGrid constructor with only mandatory points.");
    const std::vector<Time> test_times = { 0, 1, 2, 4 };
    const TimeGrid tg(test_times.begin(), test_times.end());
    
    // Time grid must include all times from passed iterator.
    // Further no additional times can be added.
    BOOST_CHECK_EQUAL_COLLECTIONS(
    	tg.begin(), tg.end(), test_times.begin(), test_times.end());
}

void TimeGridTest::testConstructorEvenSteps()
{
    BOOST_TEST_MESSAGE(
    	"Test TimeGrid construction with n evenly spaced points");
    
    Time end_time = 10;
    Size steps = 5;
    const TimeGrid tg(end_time, steps);
    const std::vector<Time> expected_times = { 0, 2, 4, 6, 8, 10 };
    
    BOOST_CHECK_EQUAL_COLLECTIONS(
    	tg.begin(), tg.end(), expected_times.begin(), expected_times.end()
    );
}

void TimeGridTest::testConstructorEmptyIterator()
{
    BOOST_TEST_MESSAGE(
    	"Test if the constructor raises an error for empty iterators."
    );
    
    const std::vector<Time> times = {};
    BOOST_CHECK_THROW(const TimeGrid tg(times.begin(), times.end()), Error);
}

void TimeGridTest::testConstructorNegativeValuesInIterator()
{
    BOOST_TEST_MESSAGE(
    	"Test if the constructor raises an error for negative time values."
    );
    
    const std::vector<Time> times = { -3, 1, 4, 5 };
    BOOST_CHECK_THROW(const TimeGrid tg(times.begin(), times.end()), Error);
}

void TimeGridTest::testClosestIndex()
{
    BOOST_TEST_MESSAGE("Test returned index is closest to the requested time."
    );
    const std::vector<Time> test_times = { 1, 2, 5 };
    const TimeGrid tg(test_times.begin(), test_times.end());
    const Size expected_index = 3;
    
    QL_ASSERT(tg.closestIndex(4) == expected_index,
    	"Expected index: " << expected_index << ", which does not match " <<
    	"the returned index: " << tg.closestIndex(4));
}

void TimeGridTest::testClosestTime()
{
    BOOST_TEST_MESSAGE("Test returned time matches to the requested index.");
    const std::vector<Time> test_times = { 1, 2, 5 };
    const TimeGrid tg(test_times.begin(), test_times.end());
    const Size expected_time = 5;
    
    QL_ASSERT(tg.closestTime(4) == expected_time,
    	"Expected time of: " << expected_time << ", which does not match " <<
    	"the returned time: " << tg.closestTime(4));
}

void TimeGridTest::testMandatoryTimes()
{
    BOOST_TEST_MESSAGE("Test mandatory times are recalled correctly.");
    const std::vector<Time> test_times = { 1, 2, 4 };
    const TimeGrid tg(test_times.begin(), test_times.end(), 8);
    
    // Mandatory times are those provided by the original iterator.
    const std::vector<Time> tg_mandatory_times = tg.mandatoryTimes();
    BOOST_CHECK_EQUAL_COLLECTIONS(
    	tg_mandatory_times.begin(), tg_mandatory_times.end(),
    	test_times.begin(), test_times.end());
}

test_suite* TimeGridTest::suite()
{
    test_suite* suite = BOOST_TEST_SUITE("Timegrid tests");
    
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testConstructorAdditionalSteps));
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testConstructorMandatorySteps));
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testConstructorEvenSteps));
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testConstructorEmptyIterator));
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testConstructorNegativeValuesInIterator));
    
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testClosestIndex));
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testClosestTime));
    suite->add(QUANTLIB_TEST_CASE(&TimeGridTest::testMandatoryTimes));
    
    return suite;
}
