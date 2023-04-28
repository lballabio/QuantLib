/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
   Copyright (C) 2018 Tom Anderson

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

#include "forwardrateagreement.hpp"
#include "utilities.hpp"
#include <ql/handle.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

void ForwardRateAgreementTest::testConstructionWithoutACurve() {
        BOOST_TEST_MESSAGE("Testing forward rate agreement construction...");

        Date today = QuantLib::Settings::instance().evaluationDate();

        // set up the index
        RelinkableHandle<YieldTermStructure> curveHandle;
        ext::shared_ptr<IborIndex> index = ext::make_shared<USDLibor>(Period(3, Months), curveHandle);

        // determine the settlement date for a FRA
        Date settlementDate = index->fixingCalendar().advance(today, index->fixingDays() * Days);

        // set up quotes with no values
        std::vector<ext::shared_ptr<SimpleQuote> > quotes = {
            ext::make_shared<SimpleQuote>(),
            ext::make_shared<SimpleQuote>(),
            ext::make_shared<SimpleQuote>()
        };

#ifdef QL_USE_INDEXED_COUPON
        bool useIndexedFra = false;
#else
        bool useIndexedFra = true;
#endif

        // set up the curve (this bit is a very rough sketch - i'm actually using swaps !)
        std::vector<ext::shared_ptr<RateHelper> > helpers;
        helpers.push_back(ext::make_shared<FraRateHelper>(Handle<Quote>(quotes[0]),
                                                          Period(1, Years), index,
                                                          Pillar::LastRelevantDate, Date(),
                                                          useIndexedFra));
        helpers.push_back(ext::make_shared<FraRateHelper>(Handle<Quote>(quotes[1]),
                                                          Period(2, Years), index,
                                                          Pillar::LastRelevantDate, Date(),
                                                          useIndexedFra));
        helpers.push_back(ext::make_shared<FraRateHelper>(Handle<Quote>(quotes[2]),
                                                          Period(3, Years), index,
                                                          Pillar::LastRelevantDate, Date(),
                                                          useIndexedFra));
        ext::shared_ptr<PiecewiseYieldCurve<ForwardRate, QuantLib::Cubic> > curve =
            ext::make_shared<PiecewiseYieldCurve<ForwardRate, QuantLib::Cubic> >(
                today, helpers, index->dayCounter());

        curveHandle.linkTo(curve);

        // set up the instrument to price
        // check the constructor without maturity date
        // inferring maturity date from the index
        ForwardRateAgreement fra(index,
                                 settlementDate + Period(12, Months),
                                 Position::Long,
                                 0,
                                 1,
                                 curveHandle);

        // finally put values in the quotes
        quotes[0]->setValue(0.01);
        quotes[1]->setValue(0.02);
        quotes[2]->setValue(0.03);

        Real rate = fra.forwardRate();
        if (std::fabs(rate - 0.01) > 1e-6) {
            BOOST_ERROR("grid creation failed for FRA without maturityDate, got rate " << rate << " expected " << 0.01);
        }

        // check the constructor with explicit maturity date
        ForwardRateAgreement fra2(index,
                                 settlementDate + Period(12, Months),
                                 settlementDate + Period(15, Months),
                                 Position::Long,
                                 0,
                                 1,
                                 curveHandle);

        Real rate2 = fra2.forwardRate();
        if (std::fabs(rate2 - 0.01) > 1e-6) {
            BOOST_ERROR("grid creation failed for FRA with maturityDate, got rate " << rate << " expected " << 0.01);
        }

}

test_suite* ForwardRateAgreementTest::suite() {
    auto* suite = BOOST_TEST_SUITE("forward rate agreement");
    suite->add(QUANTLIB_TEST_CASE(&ForwardRateAgreementTest::testConstructionWithoutACurve));
    return suite;
}
