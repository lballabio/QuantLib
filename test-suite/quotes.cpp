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

#include "quotes.hpp"
#include "utilities.hpp"
#include <ql/quotes/simplequote.hpp>
#include <ql/quotes/derivedquote.hpp>
#include <ql/quotes/compositequote.hpp>
#include <ql/quotes/forwardvaluequote.hpp>
#include <ql/quotes/impliedstddevquote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/blackformula.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace quotes_test {

    Real add10(Real x) { return x+10; }
    Real mul10(Real x) { return x*10; }
    Real sub10(Real x) { return x-10; }

    Real add(Real x, Real y) { return x+y; }
    Real mul(Real x, Real y) { return x*y; }
    Real sub(Real x, Real y) { return x-y; }

}


void QuoteTest::testObservable() {

    BOOST_TEST_MESSAGE("Testing observability of quotes...");

    std::shared_ptr<SimpleQuote> me(new SimpleQuote(0.0));
    Flag f;
    f.registerWith(me);
    me->setValue(3.14);

    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of quote change");

}

void QuoteTest::testObservableHandle() {

    BOOST_TEST_MESSAGE("Testing observability of quote handles...");

    std::shared_ptr<SimpleQuote> me1(new SimpleQuote(0.0));
    RelinkableHandle<Quote> h(me1);
    Flag f;
    f.registerWith(h);

    me1->setValue(3.14);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of quote change");

    f.lower();
    std::shared_ptr<SimpleQuote> me2(new SimpleQuote(0.0));
    h.linkTo(me2);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of quote change");

}

void QuoteTest::testDerived() {

    BOOST_TEST_MESSAGE("Testing derived quotes...");

    using namespace quotes_test;

    typedef Real (*unary_f)(Real);
    unary_f funcs[3] = { add10, mul10, sub10 };

    std::shared_ptr<Quote> me(new SimpleQuote(17.0));
    Handle<Quote> h(me);

    for (auto& func : funcs) {
        DerivedQuote<unary_f> derived(h, func);
        Real x = derived.value(), y = func(me->value());
        if (std::fabs(x-y) > 1.0e-10)
            BOOST_FAIL("derived quote yields " << x << "\n"
                       << "function result is " << y);
    }
}

void QuoteTest::testComposite() {

    BOOST_TEST_MESSAGE("Testing composite quotes...");

    typedef Real (*binary_f)(Real,Real);
    binary_f funcs[3] = { quotes_test::add, quotes_test::mul, quotes_test::sub };

    std::shared_ptr<Quote> me1(new SimpleQuote(12.0)),
                             me2(new SimpleQuote(13.0));
    Handle<Quote> h1(me1), h2(me2);

    for (auto& func : funcs) {
        CompositeQuote<binary_f> composite(h1, h2, func);
        Real x = composite.value(), y = func(me1->value(), me2->value());
        if (std::fabs(x-y) > 1.0e-10)
            BOOST_FAIL("composite quote yields " << x << "\n"
                       << "function result is " << y);
    }
}

void QuoteTest::testForwardValueQuoteAndImpliedStdevQuote(){
    BOOST_TEST_MESSAGE(
            "Testing forward-value and implied-standard-deviation quotes...");
    Real forwardRate = .05;
    DayCounter dc = ActualActual(ActualActual::ISDA);
    Calendar calendar = TARGET();
    std::shared_ptr<SimpleQuote> forwardQuote(new SimpleQuote(forwardRate));
    Handle<Quote> forwardHandle(forwardQuote);
    Date evaluationDate = Settings::instance().evaluationDate();
    std::shared_ptr<YieldTermStructure>yc (new FlatForward(
        evaluationDate, forwardHandle, dc));
    Handle<YieldTermStructure> ycHandle(yc);
    Period euriborTenor(1,Years);
    std::shared_ptr<Index> euribor(new Euribor(euriborTenor, ycHandle));
    Date fixingDate = calendar.advance(evaluationDate, euriborTenor);
    std::shared_ptr<ForwardValueQuote> forwardValueQuote( new
        ForwardValueQuote(euribor, fixingDate));
    Rate forwardValue =  forwardValueQuote->value();
    Rate expectedForwardValue = euribor->fixing(fixingDate, true);
    // we test if the forward value given by the quote is consistent
    // with the one directly given by the index
    if (std::fabs(forwardValue-expectedForwardValue) > 1.0e-15)
        BOOST_FAIL("Foward Value Quote quote yields " << forwardValue << "\n"
                   << "expected result is " << expectedForwardValue);
    // then we test the observer/observable chain
    Flag f;
    f.registerWith(forwardValueQuote);
    forwardQuote->setValue(0.04);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of quote change");

    // and we retest if the values are still matching
    forwardValue =  forwardValueQuote->value();
    expectedForwardValue = euribor->fixing(fixingDate, true);
    if (std::fabs(forwardValue-expectedForwardValue) > 1.0e-15)
        BOOST_FAIL("Foward Value Quote quote yields " << forwardValue << "\n"
                   << "expected result is " << expectedForwardValue);
    // we test the ImpliedStdevQuote class
    f.unregisterWith(forwardValueQuote);
    f.lower();
    Real price = 0.02;
    Rate strike = 0.04;
    Volatility guess = .15;
    Real accuracy = 1.0e-6;
    Option::Type optionType = Option::Call;
    std::shared_ptr<SimpleQuote> priceQuote(new SimpleQuote(price));
    Handle<Quote> priceHandle(priceQuote);
    std::shared_ptr<ImpliedStdDevQuote> impliedStdevQuote(new
        ImpliedStdDevQuote(optionType, forwardHandle, priceHandle,
                           strike, guess, accuracy));
    Real impliedStdev = impliedStdevQuote->value();
    Real expectedImpliedStdev =
        blackFormulaImpliedStdDev(optionType, strike,
                                  forwardQuote->value(), price,
                                  1.0, 0.0, guess, 1.0e-6);
    if (std::fabs(impliedStdev-expectedImpliedStdev) > 1.0e-15)
        BOOST_FAIL("\nimpliedStdevQuote yields :" << impliedStdev <<
                   "\nexpected result is       :" << expectedImpliedStdev);
    // then we test the observer/observable chain
    std::shared_ptr<Quote> quote = impliedStdevQuote;
    f.registerWith(quote);
    forwardQuote->setValue(0.05);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of quote change");
    quote->value();
    f.lower();
    quote->value();
    priceQuote->setValue(0.11);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of quote change");

}


test_suite* QuoteTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Quote tests");
    suite->add(QUANTLIB_TEST_CASE(&QuoteTest::testObservable));
    suite->add(QUANTLIB_TEST_CASE(&QuoteTest::testObservableHandle));
    suite->add(QUANTLIB_TEST_CASE(&QuoteTest::testDerived));
    suite->add(QUANTLIB_TEST_CASE(&QuoteTest::testComposite));
    suite->add(QUANTLIB_TEST_CASE(
                      &QuoteTest::testForwardValueQuoteAndImpliedStdevQuote));
    return suite;
}

