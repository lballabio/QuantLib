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

#include <ql/timegrid.hpp>
#include "toplevelfixture.hpp"
#include "utilities.hpp"

#include <iostream>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TimeGridTests)

BOOST_AUTO_TEST_CASE(testConstructorAdditionalSteps)
{
    BOOST_TEST_MESSAGE("Testing TimeGrid construction with additional steps...");

    const TimeGrid tg = {{1.0, 2.0, 4.0}, 8};

    // Expect 8 evenly sized steps over the interval [0, 4].
    std::vector<Time> expected_times = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(tg.begin(), tg.end(), expected_times.begin(),
                                  expected_times.end());
}

BOOST_AUTO_TEST_CASE(testConstructorMandatorySteps)
{
    BOOST_TEST_MESSAGE("Testing TimeGrid construction with only mandatory points...");

    const TimeGrid tg = {0.0, 1.0, 2.0, 4.0};

    // Time grid must include all times from passed iterator.
    // Further no additional times can be added.
    std::vector<Time> expected_times = {0.0, 1.0, 2.0, 4.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        tg.begin(), tg.end(), expected_times.begin(), expected_times.end());
}

BOOST_AUTO_TEST_CASE(testConstructorAdditionalStepsAutomatically)
{
    BOOST_TEST_MESSAGE("Testing TimeGrid construction with time step length determined automatically...");

    const TimeGrid tg{{0.0, 1.0, 2.0, 4.0}, 0};

    // Time step length is determined by minimal adjacent distance in given times
    const std::vector<Time> expected_times{0.0, 1.0, 2.0, 3.0, 4.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        tg.begin(), tg.end(), expected_times.begin(), expected_times.end());
}

BOOST_AUTO_TEST_CASE(testConstructorEvenSteps)
{
    BOOST_TEST_MESSAGE("Testing TimeGrid construction with n evenly spaced points...");
    
    Time end_time = 10;
    Size steps = 5;
    const TimeGrid tg(end_time, steps);

    std::vector<Time> expected_times = {0.0, 2.0, 4.0, 6.0, 8.0, 10.0};
    
    BOOST_CHECK_EQUAL_COLLECTIONS(
        tg.begin(), tg.end(), expected_times.begin(), expected_times.end()
    );
}

BOOST_AUTO_TEST_CASE(testConstructorEmptyIterator)
{
    BOOST_TEST_MESSAGE(
        "Testing that the TimeGrid constructor raises an error for empty iterators..."
    );
    
    const std::vector<Time> times;
    BOOST_CHECK_THROW(const TimeGrid tg(times.begin(), times.end()), Error);
}

BOOST_AUTO_TEST_CASE(testConstructorNegativeValuesInIterator)
{
    BOOST_TEST_MESSAGE("Testing that the TimeGrid constructor raises an error for negative time values...");
    
    std::vector<Time> times = {-3.0, 1.0, 4.0, 5.0};
    BOOST_CHECK_THROW(const TimeGrid tg(times.begin(), times.end()), Error);
}

BOOST_AUTO_TEST_CASE(testIndex)
{
    BOOST_TEST_MESSAGE("Testing that querying an index by floating-point time works for exact time nodes and "
                       "throws otherwise...");

    // will automatically insert additional point at t=0
    const TimeGrid tg = {1.0, 2.0, 5.0};

    BOOST_CHECK_THROW(tg.index(-2.0), Error);

    BOOST_ASSERT(4U == tg.size());

    BOOST_CHECK_THROW(tg.index(-0.1), Error);
    BOOST_TEST(0 == tg.index(0.0));
    BOOST_CHECK_THROW(tg.index(0.5), Error);
    BOOST_TEST(1 == tg.index(1.0));
    BOOST_CHECK_THROW(tg.index(1.1), Error);
    BOOST_TEST(2 == tg.index(2.0));
    BOOST_CHECK_THROW(tg.index(2.9), Error);
    BOOST_TEST(3 == tg.index(5.0));
    BOOST_CHECK_THROW(tg.index(5.1), Error);
}

BOOST_AUTO_TEST_CASE(testClosestIndex)
{
    BOOST_TEST_MESSAGE("Testing that the returned index is closest to the requested time...");

    const TimeGrid tg = {1.0, 2.0, 5.0};
    const Size expected_index = 3;
    
    QL_ASSERT(tg.closestIndex(4) == expected_index,
              "Expected index: " << expected_index << ", which does not match " <<
              "the returned index: " << tg.closestIndex(4));
}

BOOST_AUTO_TEST_CASE(testClosestTime)
{
    BOOST_TEST_MESSAGE("Testing that the returned time matches the requested index...");
    const TimeGrid tg = {1.0, 2.0, 5.0};
    const Size expected_time = 5;
    
    QL_ASSERT(tg.closestTime(4) == expected_time,
              "Expected time of: " << expected_time << ", which does not match " <<
              "the returned time: " << tg.closestTime(4));
}

BOOST_AUTO_TEST_CASE(testMandatoryTimes)
{
    BOOST_TEST_MESSAGE("Testing that mandatory times are recalled correctly...");
    std::vector<Time> test_times = {1.0, 2.0, 4.0};
    const TimeGrid tg(test_times.begin(), test_times.end(), 8);
    
    // Mandatory times are those provided by the original iterator.
    const std::vector<Time>& tg_mandatory_times = tg.mandatoryTimes();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        tg_mandatory_times.begin(), tg_mandatory_times.end(),
        test_times.begin(), test_times.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
