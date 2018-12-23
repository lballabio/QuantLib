/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina
 Copyright (C) 2018 StatPro Italia srl

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

#include "sofrfutures.hpp"
#include "utilities.hpp"
#include <ql/experimental/futures/overnightindexfuture.hpp>
#include <ql/experimental/futures/overnightindexfutureratehelper.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct SofrQuotes{
        Frequency freq;
        Month month;
        Year year;
        Real price;
    };

}


void SofrFuturesTest::testBootstrap() {
    BOOST_TEST_MESSAGE(
        "Testing bootstrap over SOFR futures...");

    SavedSettings backup;

    Date today = Date(26, October, 2018);
    Settings::instance().evaluationDate() = today;

    const SofrQuotes sofrQuotes[] = {
        {Monthly, Oct, 2018, 97.8175},
        {Monthly, Nov, 2018, 97.770},
        {Monthly, Dec, 2018, 97.685},
        {Monthly, Jan, 2019, 97.595},
        {Monthly, Feb, 2019, 97.590},
        {Monthly, Mar, 2019, 97.525},
        //{Quarterly, Sep, 2018, 97.8175}, // removed due to overlap in bootstrap
        //{Quarterly, Dec, 2018, 97.600},
        {Quarterly, Mar, 2019, 97.440},
        {Quarterly, Jun, 2019, 97.295},
        {Quarterly, Sep, 2019, 97.220},
        {Quarterly, Dec, 2019, 97.170},
        {Quarterly, Mar, 2020, 97.160},
        {Quarterly, Jun, 2020, 97.165},
        {Quarterly, Sep, 2020, 97.175},
    };

    ext::shared_ptr<OvernightIndex> index = ext::make_shared<Sofr>();
    index->addFixing(Date(17,October,2018), 0.0217);
    index->addFixing(Date(18,October,2018), 0.0219);
    index->addFixing(Date(19,October,2018), 0.0219);
    index->addFixing(Date(22,October,2018), 0.0218);
    index->addFixing(Date(23,October,2018), 0.0217);
    index->addFixing(Date(24,October,2018), 0.0218);
    index->addFixing(Date(25,October,2018), 0.0219);

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    for (Size i=0; i<LENGTH(sofrQuotes); i++){
        helpers.push_back(ext::make_shared<SofrFutureRateHelper>(
                 sofrQuotes[i].price, sofrQuotes[i].month, sofrQuotes[i].year,
                 sofrQuotes[i].freq, index));
    }

    ext::shared_ptr<PiecewiseYieldCurve<Discount,Linear> > curve =
        ext::make_shared<PiecewiseYieldCurve<Discount,Linear> >(today, helpers,
                                                                Actual365Fixed());

    // test curve with one of the futures
    ext::shared_ptr<Payoff> payoff(new ForwardTypePayoff(Position::Long, 97.440));
    OvernightIndexFuture sf(index, payoff,
                            Date(20, Mar, 2019), Date(19, Jun, 2019),
                            Handle<YieldTermStructure>(curve));

    Real expected_price = 97.44;
    Real expected_npv = 0.0;
    Real tolerance = 1.0e-9;

    Real error = std::fabs(sf.spotValue() - expected_price);
    if (error > tolerance) {
        BOOST_ERROR("sample futures:\n"
                    << std::setprecision(8)
                    << "\n estimated price: " << sf.spotValue()
                    << "\n expected price:  " << expected_price
                    << "\n error:           " << error
                    << "\n tolerance:       " << tolerance);
    }

    error = std::fabs(sf.NPV() - expected_npv);
    if (error > tolerance) {
        BOOST_ERROR("sample futures:\n"
                    << std::setprecision(8)
                    << "\n estimated NPV: " << sf.NPV()
                    << "\n expected NPV:  " << expected_npv
                    << "\n error:         " << error
                    << "\n tolerance:     " << tolerance);
    }

}


test_suite* SofrFuturesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("SOFR futures tests");

    suite->add(QUANTLIB_TEST_CASE(&SofrFuturesTest::testBootstrap));

    return suite;
}
