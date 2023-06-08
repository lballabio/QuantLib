/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 StatPro Italia srl

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

#include "lazyobject.hpp"
#include "utilities.hpp"
#include <ql/instruments/stock.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using ext::shared_ptr;

void LazyObjectTest::testDiscardingNotifications() {

    BOOST_TEST_MESSAGE("Testing that lazy objects can discard notifications after the first...");

    ext::shared_ptr<SimpleQuote> q(new SimpleQuote(0.0));
    ext::shared_ptr<Instrument> s(new Stock(Handle<Quote>(q)));

    Flag f;
    f.registerWith(s);

    s->forwardFirstNotificationOnly();

    s->NPV();
    q->setValue(1.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change");

    f.lower();
    q->setValue(2.0);
    if (f.isUp())
        BOOST_FAIL("Observer was notified of second change");

    f.lower();
    s->NPV();
    q->setValue(3.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change after recalculation");

    s->alwaysForwardNotifications();

    f.lower();
    s->NPV();
    q->setValue(1.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change");

    f.lower();
    q->setValue(2.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of second change");

}


void LazyObjectTest::testForwardingNotificationsByDefault() {

    BOOST_TEST_MESSAGE("Testing that lazy objects forward all notifications by default...");

    ext::shared_ptr<SimpleQuote> q(new SimpleQuote(0.0));
    ext::shared_ptr<Instrument> s(new Stock(Handle<Quote>(q)));

    Flag f;
    f.registerWith(s);

    s->NPV();
    q->setValue(1.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change");

    f.lower();
    q->setValue(2.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of second change");
}


test_suite* LazyObjectTest::suite() {
    auto* suite = BOOST_TEST_SUITE("LazyObject tests");
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testDiscardingNotifications));
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testForwardingNotificationsByDefault));
    return suite;
}
