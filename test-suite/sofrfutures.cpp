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
#include <ql/instruments/overnightindexfuture.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/overnightindexfutureratehelper.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct SofrQuotes {
        Frequency freq;
        Month month;
        Year year;
        Real price;
        RateAveraging::Type averagingMethod;
    };

}


void SofrFuturesTest::testBootstrap() {
    BOOST_TEST_MESSAGE("Testing bootstrap over SOFR futures...");

    Date today = Date(26, October, 2018);
    Settings::instance().evaluationDate() = today;

    const SofrQuotes sofrQuotes[] = {
        {Monthly, Oct, 2018, 97.8175, RateAveraging::Simple},
        {Monthly, Nov, 2018, 97.770, RateAveraging::Simple},
        {Monthly, Dec, 2018, 97.685, RateAveraging::Simple},
        {Monthly, Jan, 2019, 97.595, RateAveraging::Simple},
        {Monthly, Feb, 2019, 97.590, RateAveraging::Simple},
        {Monthly, Mar, 2019, 97.525, RateAveraging::Simple},
        {Quarterly, Mar, 2019, 97.440, RateAveraging::Compound},
        {Quarterly, Jun, 2019, 97.295, RateAveraging::Compound},
        {Quarterly, Sep, 2019, 97.220, RateAveraging::Compound},
        {Quarterly, Dec, 2019, 97.170, RateAveraging::Compound},
        {Quarterly, Mar, 2020, 97.160, RateAveraging::Compound},
        {Quarterly, Jun, 2020, 97.165, RateAveraging::Compound},
        {Quarterly, Sep, 2020, 97.175, RateAveraging::Compound},
    };

    ext::shared_ptr<OvernightIndex> index = ext::make_shared<Sofr>();
    index->addFixing(Date(1, October, 2018), 0.0222);
    index->addFixing(Date(2, October, 2018), 0.022);
    index->addFixing(Date(3, October, 2018), 0.022);
    index->addFixing(Date(4, October, 2018), 0.0218);
    index->addFixing(Date(5, October, 2018), 0.0216);
    index->addFixing(Date(9, October, 2018), 0.0215);
    index->addFixing(Date(10, October, 2018), 0.0215);
    index->addFixing(Date(11, October, 2018), 0.0217);
    index->addFixing(Date(12, October, 2018), 0.0218);
    index->addFixing(Date(15, October, 2018), 0.0221);
    index->addFixing(Date(16, October, 2018), 0.0218);
    index->addFixing(Date(17, October, 2018), 0.0218);
    index->addFixing(Date(18, October, 2018), 0.0219);
    index->addFixing(Date(19, October, 2018), 0.0219);
    index->addFixing(Date(22, October, 2018), 0.0218);
    index->addFixing(Date(23, October, 2018), 0.0217);
    index->addFixing(Date(24, October, 2018), 0.0218);
    index->addFixing(Date(25, October, 2018), 0.0219);

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    for (const auto& sofrQuote : sofrQuotes) {
        helpers.push_back(ext::make_shared<SofrFutureRateHelper>(
            sofrQuote.price, sofrQuote.month, sofrQuote.year, sofrQuote.freq));
    }

    ext::shared_ptr<PiecewiseYieldCurve<Discount, Linear> > curve =
        ext::make_shared<PiecewiseYieldCurve<Discount, Linear> >(today, helpers,
                                                                 Actual365Fixed());

    // test curve with one of the futures
    ext::shared_ptr<OvernightIndex> sofr =
        ext::make_shared<Sofr>(Handle<YieldTermStructure>(curve));
    OvernightIndexFuture sf(sofr, Date(20, March, 2019), Date(19, June, 2019));

    Real expected_price = 97.44;
    Real tolerance = 1.0e-9;

    Real error = std::fabs(sf.NPV() - expected_price);
    if (error > tolerance) {
        BOOST_ERROR("sample futures:\n"
                    << std::setprecision(8)
                    << "\n estimated price: " << sf.NPV()
                    << "\n expected price:  " << expected_price
                    << "\n error:           " << error
                    << "\n tolerance:       " << tolerance);
    }
}


test_suite* SofrFuturesTest::suite() {
    auto* suite = BOOST_TEST_SUITE("SOFR futures tests");

    suite->add(QUANTLIB_TEST_CASE(&SofrFuturesTest::testBootstrap));

    return suite;
}
