/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

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
#include <ql/settings.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SettingsTests)

BOOST_AUTO_TEST_CASE(testNotificationsOnDateChange) {
    BOOST_TEST_MESSAGE("Testing notifications on evaluation-date change...");

#ifdef QL_HIGH_RESOLUTION_DATE

    Date d1(11, February, 2021, 9, 17, 0);
    Date d2(11, February, 2021, 10, 21, 0);

#else

    Date d1(11, February, 2021);
    Date d2(12, February, 2021);

#endif

    Settings::instance().evaluationDate() = d1;

    Flag flag;
    flag.registerWith(Settings::instance().evaluationDate());

    // Set to same date, no notification
    Settings::instance().evaluationDate() = d1;

    if (flag.isUp())
        BOOST_ERROR("unexpected notification");

    // Set to different date, notification expected
    Settings::instance().evaluationDate() = d2;

    if (!flag.isUp())
        BOOST_ERROR("missing notification");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
