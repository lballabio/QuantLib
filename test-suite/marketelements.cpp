
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

#include "marketelements.hpp"
#include "utilities.hpp"
#include <ql/marketelement.hpp>
#include <ql/dataformatters.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    double add10(double x) { return x+10; }
    double mul10(double x) { return x*10; }
    double sub10(double x) { return x-10; }

    double add(double x, double y) { return x+y; }
    double mul(double x, double y) { return x*y; }
    double sub(double x, double y) { return x-y; }

}

void MarketElementTest::testObservable() {

    boost::shared_ptr<SimpleQuote> me(new SimpleQuote(0.0));
    Flag f;
    f.registerWith(me);
    me->setValue(3.14);

    if (!f.isUp())
        CPPUNIT_FAIL("Observer was not notified of market element change");

}

void MarketElementTest::testObservableHandle() {

    boost::shared_ptr<SimpleQuote> me1(new SimpleQuote(0.0));
    RelinkableHandle<Quote> h(me1);
    Flag f;
    f.registerWith(h);

    me1->setValue(3.14);
    if (!f.isUp())
        CPPUNIT_FAIL("Observer was not notified of market element change");

    f.lower();
    boost::shared_ptr<SimpleQuote> me2(new SimpleQuote(0.0));
    h.linkTo(me2);
    if (!f.isUp())
        CPPUNIT_FAIL("Observer was not notified of market element change");

}

void MarketElementTest::testDerived() {

    typedef double (*unary_f)(double);
    unary_f funcs[3] = { add10, mul10, sub10 };

    boost::shared_ptr<Quote> me(new SimpleQuote(17.0));
    RelinkableHandle<Quote> h(me);

    for (int i=0; i<3; i++) {
        DerivedQuote<unary_f> derived(h,funcs[i]);
        double x = derived.value(),
               y = funcs[i](me->value());
        if (QL_FABS(x-y) > 1.0e-10)
            CPPUNIT_FAIL(
                "derived market element yields " +
                DoubleFormatter::toString(x) + "\n"
                "function result is " +
                DoubleFormatter::toString(y));
    }
}

void MarketElementTest::testComposite() {

    typedef double (*binary_f)(double,double);
    binary_f funcs[3] = { add, mul, sub };

    boost::shared_ptr<Quote> me1(new SimpleQuote(12.0)),
                             me2(new SimpleQuote(13.0));
    RelinkableHandle<Quote> h1(me1), h2(me2);

    for (int i=0; i<3; i++) {
        CompositeQuote<binary_f> composite(h1,h2,funcs[i]);
        double x = composite.value(),
               y = funcs[i](me1->value(),me2->value());
        if (QL_FABS(x-y) > 1.0e-10)
            CPPUNIT_FAIL(
                "composite market element yields " +
                DoubleFormatter::toString(x) + "\n"
                "function result is " +
                DoubleFormatter::toString(y));
    }
}

CppUnit::Test* MarketElementTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Market element tests");
    tests->addTest(new CppUnit::TestCaller<MarketElementTest>
                   ("Testing observability of market elements",
                    &MarketElementTest::testObservable));
    tests->addTest(new CppUnit::TestCaller<MarketElementTest>
                   ("Testing observability of market element handles",
                    &MarketElementTest::testObservableHandle));
    tests->addTest(new CppUnit::TestCaller<MarketElementTest>
                   ("Testing derived market elements",
                    &MarketElementTest::testDerived));
    tests->addTest(new CppUnit::TestCaller<MarketElementTest>
                   ("Testing composite market elements",
                    &MarketElementTest::testComposite));
    return tests;
}

