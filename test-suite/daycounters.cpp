/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2006 Piter Dias
 Copyright (C) 2012 Simon Shakeshaft

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

#include "daycounters.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/one.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/period.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct SingleCase {
        SingleCase(ActualActual::Convention convention,
                   const Date& start,
                   const Date& end,
                   const Date& refStart,
                   const Date& refEnd,
                   Time result)
        : convention(convention), start(start), end(end),
          refStart(refStart), refEnd(refEnd), result(result) {}
        SingleCase(ActualActual::Convention convention,
                   const Date& start,
                   const Date& end,
                   Time result)
        : convention(convention), start(start), end(end),
          refStart(Date()), refEnd(Date()), result(result) {}
        ActualActual::Convention convention;
        Date start;
        Date end;
        Date refStart;
        Date refEnd;
        Time result;
    };

}


void DayCounterTest::testActualActual() {

    BOOST_TEST_MESSAGE("Testing actual/actual day counters...");

    SingleCase testCases[] = {
        // first example
        SingleCase(ActualActual::ISDA,
                   Date(1,November,2003), Date(1,May,2004),
                   0.497724380567),
        SingleCase(ActualActual::ISMA,
                   Date(1,November,2003), Date(1,May,2004),
                   Date(1,November,2003), Date(1,May,2004),
                   0.500000000000),
        SingleCase(ActualActual::AFB,
                   Date(1,November,2003), Date(1,May,2004),
                   0.497267759563),
        // short first calculation period (first period)
        SingleCase(ActualActual::ISDA,
                   Date(1,February,1999), Date(1,July,1999),
                   0.410958904110),
        SingleCase(ActualActual::ISMA,
                   Date(1,February,1999), Date(1,July,1999),
                   Date(1,July,1998), Date(1,July,1999),
                   0.410958904110),
        SingleCase(ActualActual::AFB,
                   Date(1,February,1999), Date(1,July,1999),
                   0.410958904110),
        // short first calculation period (second period)
        SingleCase(ActualActual::ISDA,
                   Date(1,July,1999), Date(1,July,2000),
                   1.001377348600),
        SingleCase(ActualActual::ISMA,
                   Date(1,July,1999), Date(1,July,2000),
                   Date(1,July,1999), Date(1,July,2000),
                   1.000000000000),
        SingleCase(ActualActual::AFB,
                   Date(1,July,1999), Date(1,July,2000),
                   1.000000000000),
        // long first calculation period (first period)
        SingleCase(ActualActual::ISDA,
                   Date(15,August,2002), Date(15,July,2003),
                   0.915068493151),
        SingleCase(ActualActual::ISMA,
                   Date(15,August,2002), Date(15,July,2003),
                   Date(15,January,2003), Date(15,July,2003),
                   0.915760869565),
        SingleCase(ActualActual::AFB,
                   Date(15,August,2002), Date(15,July,2003),
                   0.915068493151),
        // long first calculation period (second period)
        /* Warning: the ISDA case is in disagreement with mktc1198.pdf */
        SingleCase(ActualActual::ISDA,
                   Date(15,July,2003), Date(15,January,2004),
                   0.504004790778),
        SingleCase(ActualActual::ISMA,
                   Date(15,July,2003), Date(15,January,2004),
                   Date(15,July,2003), Date(15,January,2004),
                   0.500000000000),
        SingleCase(ActualActual::AFB,
                   Date(15,July,2003), Date(15,January,2004),
                   0.504109589041),
        // short final calculation period (penultimate period)
        SingleCase(ActualActual::ISDA,
                   Date(30,July,1999), Date(30,January,2000),
                   0.503892506924),
        SingleCase(ActualActual::ISMA,
                   Date(30,July,1999), Date(30,January,2000),
                   Date(30,July,1999), Date(30,January,2000),
                   0.500000000000),
        SingleCase(ActualActual::AFB,
                   Date(30,July,1999), Date(30,January,2000),
                   0.504109589041),
        // short final calculation period (final period)
        SingleCase(ActualActual::ISDA,
                   Date(30,January,2000), Date(30,June,2000),
                   0.415300546448),
        SingleCase(ActualActual::ISMA,
                   Date(30,January,2000), Date(30,June,2000),
                   Date(30,January,2000), Date(30,July,2000),
                   0.417582417582),
        SingleCase(ActualActual::AFB,
                   Date(30,January,2000), Date(30,June,2000),
                   0.41530054644)
    };

    Size n = sizeof(testCases)/sizeof(SingleCase);
    for (Size i=0; i<n; i++) {
        ActualActual dayCounter(testCases[i].convention);
        Date d1 = testCases[i].start,
            d2 = testCases[i].end,
            rd1 = testCases[i].refStart,
            rd2 = testCases[i].refEnd;
        Time calculated = dayCounter.yearFraction(d1,d2,rd1,rd2);

        if (std::fabs(calculated-testCases[i].result) > 1.0e-10) {
            std::ostringstream period, refPeriod;
            period << "period: " << d1 << " to " << d2;
            if (testCases[i].convention == ActualActual::ISMA)
                refPeriod << "referencePeriod: " << rd1 << " to " << rd2;
            BOOST_FAIL(dayCounter.name() << ":\n"
                       << period.str() << "\n" << refPeriod.str() << "\n"
                       << std::setprecision(10)
                       << "    calculated: " << calculated << "\n"
                       << "    expected:   " << testCases[i].result);
        }
    }
}


void DayCounterTest::testSimple() {

    BOOST_TEST_MESSAGE("Testing simple day counter...");

    Period p[] = { Period(3,Months), Period(6,Months), Period(1,Years) };
    Time expected[] = { 0.25, 0.5, 1.0 };
    Size n = sizeof(p)/sizeof(Period);

    // 4 years should be enough
    Date first(1,January,2002), last(31,December,2005);
    DayCounter dayCounter = SimpleDayCounter();

    for (Date start = first; start <= last; start++) {
        for (Size i=0; i<n; i++) {
            Date end = start + p[i];
            Time calculated = dayCounter.yearFraction(start,end);
            if (std::fabs(calculated-expected[i]) > 1.0e-12) {
                BOOST_FAIL("from " << start << " to " << end << ":\n"
                           << std::setprecision(12)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected[i]);
            }
        }
    }
}

void DayCounterTest::testOne() {

    BOOST_TEST_MESSAGE("Testing 1/1 day counter...");

    Period p[] = { Period(3,Months), Period(6,Months), Period(1,Years) };
    Time expected[] = { 1.0, 1.0, 1.0 };
    Size n = sizeof(p)/sizeof(Period);

    // 1 years should be enough
    Date first(1,January,2004), last(31,December,2004);
    DayCounter dayCounter = OneDayCounter();

    for (Date start = first; start <= last; start++) {
        for (Size i=0; i<n; i++) {
            Date end = start + p[i];
            Time calculated = dayCounter.yearFraction(start,end);
            if (std::fabs(calculated-expected[i]) > 1.0e-12) {
                BOOST_FAIL("from " << start << " to " << end << ":\n"
                           << std::setprecision(12)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected[i]);
            }
        }
    }
}

void DayCounterTest::testBusiness252() {

    BOOST_TEST_MESSAGE("Testing business/252 day counter...");

    std::vector<Date> testDates;
    testDates.push_back(Date(1,February,2002));
    testDates.push_back(Date(4,February,2002));
    testDates.push_back(Date(16,May,2003));
    testDates.push_back(Date(17,December,2003));
    testDates.push_back(Date(17,December,2004));
    testDates.push_back(Date(19,December,2005));
    testDates.push_back(Date(2,January,2006));
    testDates.push_back(Date(13,March,2006));
    testDates.push_back(Date(15,May,2006));
    testDates.push_back(Date(17,March,2006));
    testDates.push_back(Date(15,May,2006));
    testDates.push_back(Date(26,July,2006));
    testDates.push_back(Date(28,June,2007));
    testDates.push_back(Date(16,September,2009));
    testDates.push_back(Date(26,July,2016));

    Time expected[] = {
        0.0039682539683,
        1.2738095238095,
        0.6031746031746,
        0.9960317460317,
        1.0000000000000,
        0.0396825396825,
        0.1904761904762,
        0.1666666666667,
        -0.1507936507937,
        0.1507936507937,
        0.2023809523810,
        0.912698412698,
        2.214285714286,
        6.84126984127
        };

    DayCounter dayCounter1 = Business252(Brazil());

    Time calculated;

    for (Size i=1; i<testDates.size(); i++) {
        calculated = dayCounter1.yearFraction(testDates[i-1],testDates[i]);
        if (std::fabs(calculated-expected[i-1]) > 1.0e-12) {
                BOOST_ERROR("from " << testDates[i-1]
                            << " to " << testDates[i] << ":\n"
                            << std::setprecision(14)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected[i-1]);
        }
    }

    DayCounter dayCounter2 = Business252();

    for (Size i=1; i<testDates.size(); i++) {
        calculated = dayCounter2.yearFraction(testDates[i-1],testDates[i]);
        if (std::fabs(calculated-expected[i-1]) > 1.0e-12) {
                BOOST_ERROR("from " << testDates[i-1]
                            << " to " << testDates[i] << ":\n"
                            << std::setprecision(14)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected[i-1]);
        }
    }
}

void DayCounterTest::testThirty360_BondBasis() {

    BOOST_TEST_MESSAGE("Testing thirty/360 day counter (Bond Basis)...");

    // http://www.isda.org/c_and_a/docs/30-360-2006ISDADefs.xls
    // Source: 2006 ISDA Definitions, Sec. 4.16 (f)
    // 30/360 (or Bond Basis)

    DayCounter dayCounter = Thirty360(Thirty360::BondBasis);
    std::vector<Date> testStartDates;
    std::vector<Date> testEndDates;
    Time calculated;

    // ISDA - Example 1: End dates do not involve the last day of February
    testStartDates.push_back(Date(20, August, 2006)); testEndDates.push_back(Date(20, February, 2007));
    testStartDates.push_back(Date(20, February, 2007)); testEndDates.push_back(Date(20, August, 2007));
    testStartDates.push_back(Date(20, August, 2007)); testEndDates.push_back(Date(20, February, 2008));
    testStartDates.push_back(Date(20, February, 2008)); testEndDates.push_back(Date(20, August, 2008));
    testStartDates.push_back(Date(20, August, 2008)); testEndDates.push_back(Date(20, February, 2009));
    testStartDates.push_back(Date(20, February, 2009)); testEndDates.push_back(Date(20, August, 2009));

    // ISDA - Example 2: End dates include some end-February dates
    testStartDates.push_back(Date(31, August, 2006)); testEndDates.push_back(Date(28, February, 2007));
    testStartDates.push_back(Date(28, February, 2007)); testEndDates.push_back(Date(31, August, 2007));
    testStartDates.push_back(Date(31, August, 2007)); testEndDates.push_back(Date(29, February, 2008));
    testStartDates.push_back(Date(29, February, 2008)); testEndDates.push_back(Date(31, August, 2008));
    testStartDates.push_back(Date(31, August, 2008)); testEndDates.push_back(Date(28, February, 2009));
    testStartDates.push_back(Date(28, February, 2009)); testEndDates.push_back(Date(31, August, 2009));

    //// ISDA - Example 3: Miscellaneous calculations
    testStartDates.push_back(Date(31, January, 2006)); testEndDates.push_back(Date(28, February, 2006));
    testStartDates.push_back(Date(30, January, 2006)); testEndDates.push_back(Date(28, February, 2006));
    testStartDates.push_back(Date(28, February, 2006)); testEndDates.push_back(Date(3, March, 2006));
    testStartDates.push_back(Date(14, February, 2006)); testEndDates.push_back(Date(28, February, 2006));
    testStartDates.push_back(Date(30, September, 2006)); testEndDates.push_back(Date(31, October, 2006));
    testStartDates.push_back(Date(31, October, 2006)); testEndDates.push_back(Date(28, November, 2006));
    testStartDates.push_back(Date(31, August, 2007)); testEndDates.push_back(Date(28, February, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(28, August, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(30, August, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(31, August, 2008));
    testStartDates.push_back(Date(26, February, 2007)); testEndDates.push_back(Date(28, February, 2008));
    testStartDates.push_back(Date(26, February, 2007)); testEndDates.push_back(Date(29, February, 2008));
    testStartDates.push_back(Date(29, February, 2008)); testEndDates.push_back(Date(28, February, 2009));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(30, March, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(31, March, 2008));

    int expected[] = { 180, 180, 180, 180, 180, 180,
                       178, 183, 179, 182, 178, 183,
                        28,  28,   5,  14,  30,  28,
                       178, 180, 182, 183, 362, 363,
                       359,  32,  33};

    for (Size i = 0; i < testStartDates.size(); i++) {
        calculated = dayCounter.dayCount(testStartDates[i], testEndDates[i]);
        if (calculated != expected[i]) {
                BOOST_ERROR("from " << testStartDates[i]
                            << " to " << testEndDates[i] << ":\n"
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected[i-1]);
        }
    }
}

void DayCounterTest::testThirty360_EurobondBasis() {

    BOOST_TEST_MESSAGE("Testing thirty/360 day counter (Eurobond Basis)...");

    // Source: ISDA 2006 Definitions 4.16 (g)
    // 30E/360 (or Eurobond Basis)
    // Based on ICMA (Rule 251) and FBF; this is the version of 30E/360 used by Excel

    DayCounter dayCounter = Thirty360(Thirty360::EurobondBasis);
    std::vector<Date> testStartDates;
    std::vector<Date> testEndDates;
    Time calculated;

    // ISDA - Example 1: End dates do not involve the last day of February
    testStartDates.push_back(Date(20, August, 2006)); testEndDates.push_back(Date(20, February, 2007));
    testStartDates.push_back(Date(20, February, 2007)); testEndDates.push_back(Date(20, August, 2007));
    testStartDates.push_back(Date(20, August, 2007)); testEndDates.push_back(Date(20, February, 2008));
    testStartDates.push_back(Date(20, February, 2008)); testEndDates.push_back(Date(20, August, 2008));
    testStartDates.push_back(Date(20, August, 2008)); testEndDates.push_back(Date(20, February, 2009));
    testStartDates.push_back(Date(20, February, 2009)); testEndDates.push_back(Date(20, August, 2009));

    //// ISDA - Example 2: End dates include some end-February dates
    testStartDates.push_back(Date(28, February, 2006)); testEndDates.push_back(Date(31, August, 2006));
    testStartDates.push_back(Date(31, August, 2006)); testEndDates.push_back(Date(28, February, 2007));
    testStartDates.push_back(Date(28, February, 2007)); testEndDates.push_back(Date(31, August, 2007));
    testStartDates.push_back(Date(31, August, 2007)); testEndDates.push_back(Date(29, February, 2008));
    testStartDates.push_back(Date(29, February, 2008)); testEndDates.push_back(Date(31, August, 2008));
    testStartDates.push_back(Date(31, August, 2008)); testEndDates.push_back(Date(28, Feb, 2009));
    testStartDates.push_back(Date(28, February, 2009)); testEndDates.push_back(Date(31, August, 2009));
    testStartDates.push_back(Date(31, August, 2009)); testEndDates.push_back(Date(28, Feb, 2010));
    testStartDates.push_back(Date(28, February, 2010)); testEndDates.push_back(Date(31, August, 2010));
    testStartDates.push_back(Date(31, August, 2010)); testEndDates.push_back(Date(28, Feb, 2011));
    testStartDates.push_back(Date(28, February, 2011)); testEndDates.push_back(Date(31, August, 2011));
    testStartDates.push_back(Date(31, August, 2011)); testEndDates.push_back(Date(29, Feb, 2012));

    //// ISDA - Example 3: Miscellaneous calculations
    testStartDates.push_back(Date(31, January, 2006)); testEndDates.push_back(Date(28, February, 2006));
    testStartDates.push_back(Date(30, January, 2006)); testEndDates.push_back(Date(28, February, 2006));
    testStartDates.push_back(Date(28, February, 2006)); testEndDates.push_back(Date(3, March, 2006));
    testStartDates.push_back(Date(14, February, 2006)); testEndDates.push_back(Date(28, February, 2006));
    testStartDates.push_back(Date(30, September, 2006)); testEndDates.push_back(Date(31, October, 2006));
    testStartDates.push_back(Date(31, October, 2006)); testEndDates.push_back(Date(28, November, 2006));
    testStartDates.push_back(Date(31, August, 2007)); testEndDates.push_back(Date(28, February, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(28, August, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(30, August, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(31, August, 2008));
    testStartDates.push_back(Date(26, February, 2007)); testEndDates.push_back(Date(28, February, 2008));
    testStartDates.push_back(Date(26, February, 2007)); testEndDates.push_back(Date(29, February, 2008));
    testStartDates.push_back(Date(29, February, 2008)); testEndDates.push_back(Date(28, February, 2009));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(30, March, 2008));
    testStartDates.push_back(Date(28, February, 2008)); testEndDates.push_back(Date(31, March, 2008));

    int expected[] = { 180, 180, 180, 180, 180, 180,
                       182, 178, 182, 179, 181, 178,
                       182, 178, 182, 178, 182, 179,
                        28,  28,   5,  14,  30,  28,
                       178, 180, 182, 182, 362, 363,
                       359,  32,  32 };

    for (Size i = 0; i < testStartDates.size(); i++) {
        calculated = dayCounter.dayCount(testStartDates[i], testEndDates[i]);
        if (calculated != expected[i]) {
                BOOST_ERROR("from " << testStartDates[i]
                            << " to " << testEndDates[i] << ":\n"
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected[i]);
        }
    }
}

test_suite* DayCounterTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Day counter tests");
    suite->add(QUANTLIB_TEST_CASE(&DayCounterTest::testActualActual));
    suite->add(QUANTLIB_TEST_CASE(&DayCounterTest::testSimple));
    suite->add(QUANTLIB_TEST_CASE(&DayCounterTest::testOne));
    suite->add(QUANTLIB_TEST_CASE(&DayCounterTest::testBusiness252));
    suite->add(QUANTLIB_TEST_CASE(&DayCounterTest::testThirty360_BondBasis));
    suite->add(QUANTLIB_TEST_CASE(&DayCounterTest::testThirty360_EurobondBasis));
    return suite;
}



