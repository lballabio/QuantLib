/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl

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

#include "issuer.hpp"
#include "utilities.hpp"
#include <ql/experimental/credit/issuer.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void IssuerTest::testObservability() {

    BOOST_MESSAGE("Testing observability of issuers...");

    boost::shared_ptr<SimpleQuote> hazardRate(new SimpleQuote(0.0100));

    Handle<DefaultProbabilityTermStructure> probability(
        boost::shared_ptr<DefaultProbabilityTermStructure>(
             new FlatHazardRate(Handle<Quote>(hazardRate), Actual360())));
    Real recoveryRate = 0.4;

    Issuer issuer(probability, recoveryRate);

    Flag f;
    f.registerWith(issuer);

    hazardRate->setValue(0.0150);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of issuer change");

}


test_suite* IssuerTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Issuer tests");
    suite->add(QUANTLIB_TEST_CASE(&IssuerTest::testObservability));
    return suite;
}

