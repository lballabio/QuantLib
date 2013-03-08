/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

#include "instruments.hpp"
#include "utilities.hpp"
#include <ql/instruments/stock.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void InstrumentTest::testObservable() {

    BOOST_TEST_MESSAGE("Testing observability of instruments...");

    boost::shared_ptr<SimpleQuote> me1(new SimpleQuote(0.0));
    RelinkableHandle<Quote> h(me1);
    boost::shared_ptr<Instrument> s(new Stock(h));

    Flag f;
    f.registerWith(s);
    
    s->NPV();
    me1->setValue(3.14);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");
    
    s->NPV();
    f.lower();
    boost::shared_ptr<SimpleQuote> me2(new SimpleQuote(0.0));
    h.linkTo(me2);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");

    f.lower();
    s->freeze();
    s->NPV();
    me2->setValue(2.71);
    if (f.isUp())
        BOOST_FAIL("Observer was notified of frozen instrument change");
    s->NPV();
    s->unfreeze();
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");
}


test_suite* InstrumentTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Instrument tests");
    suite->add(QUANTLIB_TEST_CASE(&InstrumentTest::testObservable));
    return suite;
}

