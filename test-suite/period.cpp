/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2014 Ferdinando Ametrano

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

#include "period.hpp"
#include "utilities.hpp"
#include "ql/time/period.hpp"

using namespace QuantLib;
using namespace boost::unit_test_framework;

void PeriodTest::testYearsMonthsAlgebra() {

    BOOST_TEST_MESSAGE("Testing period algebra on years/months...");

    Period OneYear(1, Years);
    Period SixMonths(6, Months);
    Period ThreeMonths(3, Months);

    Integer n = 4;
    if (OneYear/n!=ThreeMonths)
        BOOST_ERROR("division error: " << OneYear << "/" << n <<
                    " not equal to " << ThreeMonths);
    n = 2;
    if (OneYear/n!=SixMonths)
        BOOST_ERROR("division error: " << OneYear << "/" << n <<
                    " not equal to " << SixMonths);

    Period sum=ThreeMonths;
    sum+=SixMonths;
    if (sum!=Period(9, Months))
        BOOST_ERROR("sum error: " << ThreeMonths <<
                    " + " << SixMonths <<
                    " != " << Period(9, Months));

    sum+=OneYear;
    if (sum!=Period(21, Months))
        BOOST_ERROR("sum error: " << ThreeMonths <<
                    " + " << SixMonths <<
                    " + " << OneYear <<
                    " != " << Period(21, Months));

    Period TwelveMonths(12, Months);
    if (TwelveMonths.length()!=12)
        BOOST_ERROR("normalization error: TwelveMonths.length()" <<
                    " is " << TwelveMonths.length() <<
                    " instead of 12");
    if (TwelveMonths.units()!=Months)
        BOOST_ERROR("normalization error: TwelveMonths.units()" <<
                    " is " << TwelveMonths.units() <<
                    " instead of " << Months);

    Period NormalizedTwelveMonths(12, Months);
    NormalizedTwelveMonths.normalize();
    if (NormalizedTwelveMonths.length()!=1)
        BOOST_ERROR("normalization error: NormalizedTwelveMonths.length()" <<
                    " is " << NormalizedTwelveMonths.length() <<
                    " instead of 1");
    if (NormalizedTwelveMonths.units()!=Years)
        BOOST_ERROR("normalization error: NormalizedTwelveMonths.units()" <<
                    " is " << NormalizedTwelveMonths.units() <<
                    " instead of " << Years);
}

void PeriodTest::testWeeksDaysAlgebra() {

    BOOST_TEST_MESSAGE("Testing period algebra on weeks/days...");

    Period TwoWeeks(2, Weeks);
    Period OneWeek(1, Weeks);
    Period ThreeDays(3, Days);
    Period OneDay(1, Days);

    Integer n = 2;
    if (TwoWeeks/n!=OneWeek)
        BOOST_ERROR("division error: " << TwoWeeks << "/" << n <<
                    " not equal to " << OneWeek);
    n = 7;
    if (OneWeek/n!=OneDay)
        BOOST_ERROR("division error: " << OneWeek << "/" << n <<
                    " not equal to " << OneDay);

    Period sum=ThreeDays;
    sum+=OneDay;
    if (sum!=Period(4, Days))
        BOOST_ERROR("sum error: " << ThreeDays <<
                    " + " << OneDay <<
                    " != " << Period(4, Days));

    sum+=OneWeek;
    if (sum!=Period(11, Days))
        BOOST_ERROR("sum error: " << ThreeDays <<
                    " + " << OneDay <<
                    " + " << OneWeek <<
                    " != " << Period(11, Days));

    Period SevenDays(7, Days);
    if (SevenDays.length()!=7)
        BOOST_ERROR("normalization error: SevenDays.length()" <<
                    " is " << SevenDays.length() <<
                    " instead of 7");
    if (SevenDays.units()!=Days)
        BOOST_ERROR("normalization error: SevenDays.units()" <<
                    " is " << SevenDays.units() <<
                    " instead of " << Days);
}

void PeriodTest::testNormalization() {

    BOOST_TEST_MESSAGE("Testing period normalization...");

    Period test_values[] = {
        0 * Days,
        0 * Weeks,
        0 * Months,
        0 * Years,
        3 * Days,
        7 * Days,
        14 * Days,
        30 * Days,
        60 * Days,
        365 * Days,
        1 * Weeks,
        2 * Weeks,
        4 * Weeks,
        8 * Weeks,
        52 * Weeks,
        1 * Months,
        2 * Months,
        6 * Months,
        12 * Months,
        18 * Months,
        24 * Months,
        1 * Years,
        2 * Years
    };

    for (Period p1 : test_values) {
        auto n1 = p1.normalized();
        if (n1 != p1) {
            BOOST_ERROR("Normalizing " << p1 << " yields " << n1 << ", which compares different");
        }

        for (Period p2 : test_values) {
            auto n2 = p2.normalized();
            ext::optional<bool> comparison;
            try {
                comparison = (p1 == p2);
            } catch (Error&) {
                ;
            }

            if (comparison && *comparison) {
                // periods which compare equal must normalize to exactly the same period
                if (n1.units() != n2.units() || n1.length() != n2.length()) {
                    BOOST_ERROR(p1 << " and " << p2 << " compare equal, but normalize to "
                                << n1 << " and " << n2 << " respectively");
                }
            }

            if (n1.units() == n2.units() && n1.length() == n2.length()) {
                // periods normalizing to exactly the same period must compare equal
                if (p1 != p2) {
                    BOOST_ERROR(p1 << " and " << p2 << " compare different, but normalize to "
                                << n1 << " and " << n2 << " respectively");
                }
            }
        }
    }

}

test_suite* PeriodTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Period tests");
    suite->add(QUANTLIB_TEST_CASE(&PeriodTest::testYearsMonthsAlgebra));
    suite->add(QUANTLIB_TEST_CASE(&PeriodTest::testWeeksDaysAlgebra));
    suite->add(QUANTLIB_TEST_CASE(&PeriodTest::testNormalization));
    return suite;
}

