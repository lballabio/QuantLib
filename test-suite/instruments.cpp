
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "instruments.hpp"
#include "utilities.hpp"
#include <ql/Instruments/stock.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

void InstrumentTest::testObservable() {

    Handle<SimpleQuote> me1(new SimpleQuote(0.0));
    RelinkableHandle<Quote> h(me1);
    Handle<Instrument> s(new Stock(h));

    Flag f;
    f.registerWith(s);

    me1->setValue(3.14);
    if (!f.isUp())
        CPPUNIT_FAIL("Observer was not notified of instrument change");

    f.lower();
    Handle<SimpleQuote> me2(new SimpleQuote(0.0));
    h.linkTo(me2);
    if (!f.isUp())
        CPPUNIT_FAIL("Observer was not notified of instrument change");

    f.lower();
    s->freeze();
    me2->setValue(2.71);
    if (f.isUp())
        CPPUNIT_FAIL("Observer was notified of frozen instrument change");
    s->unfreeze();
    if (!f.isUp())
        CPPUNIT_FAIL("Observer was not notified of instrument change");
}

CppUnit::Test* InstrumentTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Instrument tests");
    tests->addTest(new CppUnit::TestCaller<InstrumentTest>
                   ("Testing observability of instruments",
                    &InstrumentTest::testObservable));
    return tests;
}

