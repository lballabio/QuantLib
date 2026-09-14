/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Abhay Chaudhary

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
#include <ql/experimental/volatility/extendedblackvariancesurface.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ExtendedBlackVarianceSurfaceTests)

BOOST_AUTO_TEST_CASE(testVariancesAtNodes) {

    BOOST_TEST_MESSAGE("Testing extended Black variance surface at its nodes...");

    Date today(15, September, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    // more than two strikes, so that a wrong stride into the volatility
    // vector reads past its end
    std::vector<Date> dates = {today + 1*Years, today + 2*Years};
    std::vector<Real> strikes = {90.0, 100.0, 110.0};

    std::vector<Handle<Quote> > volatilities;
    for (Size i=0; i<strikes.size(); ++i)
        for (Size j=0; j<dates.size(); ++j)
            volatilities.emplace_back(
                ext::make_shared<SimpleQuote>(0.10 + 0.10*i + 0.01*j));

    ExtendedBlackVarianceSurface surface(today, NullCalendar(), dates, strikes,
                                         volatilities, dc);

    for (Size i=0; i<strikes.size(); ++i) {
        for (Size j=0; j<dates.size(); ++j) {
            Time t = dc.yearFraction(today, dates[j]);
            Volatility sigma = 0.10 + 0.10*i + 0.01*j;
            Real expected = t * sigma * sigma;
            Real calculated = surface.blackVariance(dates[j], strikes[i]);
            if (std::fabs(calculated - expected) > 1.0e-10)
                BOOST_ERROR("failed to reproduce variance at a node"
                            << "\n    strike:     " << strikes[i]
                            << "\n    time:       " << t
                            << "\n    calculated: " << calculated
                            << "\n    expected:   " << expected);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
