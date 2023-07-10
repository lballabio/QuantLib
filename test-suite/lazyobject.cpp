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

namespace lazy_object_test {

    class TearDown {
        bool alwaysForward;
      public:
        TearDown() : alwaysForward(LazyObjectSettings::instance().forwardsAllNotifications()) {}
        ~TearDown() {
            if (alwaysForward)
                LazyObjectSettings::instance().alwaysForwardNotifications();
            else
                LazyObjectSettings::instance().forwardFirstNotificationOnly();
        }
    };

}

void LazyObjectTest::testDiscardingNotifications() {

    BOOST_TEST_MESSAGE("Testing that lazy objects can discard notifications after the first against default...");

    lazy_object_test::TearDown teardown;

    LazyObjectSettings::instance().alwaysForwardNotifications();

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
}


void LazyObjectTest::testDiscardingNotificationsByDefault() {

    BOOST_TEST_MESSAGE("Testing that lazy objects can discard notifications after the first by default...");

    lazy_object_test::TearDown teardown;

    LazyObjectSettings::instance().forwardFirstNotificationOnly();

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
    if (f.isUp())
        BOOST_FAIL("Observer was notified of second change");

    f.lower();
    s->NPV();
    q->setValue(3.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change after recalculation");
}


void LazyObjectTest::testForwardingNotificationsByDefault() {

    BOOST_TEST_MESSAGE("Testing that lazy objects can forward all notifications by default...");

    lazy_object_test::TearDown teardown;

    LazyObjectSettings::instance().alwaysForwardNotifications();

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

void LazyObjectTest::testForwardingNotifications() {

    BOOST_TEST_MESSAGE("Testing that lazy objects can forward all notifications against default...");

    lazy_object_test::TearDown teardown;

    LazyObjectSettings::instance().forwardFirstNotificationOnly();

    ext::shared_ptr<SimpleQuote> q(new SimpleQuote(0.0));
    ext::shared_ptr<Instrument> s(new Stock(Handle<Quote>(q)));

    Flag f;
    f.registerWith(s);

    s->alwaysForwardNotifications();

    s->NPV();
    q->setValue(1.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change");

    f.lower();
    q->setValue(2.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of second change");
}

void LazyObjectTest::testNotificationLoop() {

    BOOST_TEST_MESSAGE("Testing that lazy objects manage recursive notifications...");

    lazy_object_test::TearDown teardown;

    LazyObjectSettings::instance().alwaysForwardNotifications();

    auto q = ext::make_shared<SimpleQuote>(0.0);
    auto s1 = ext::make_shared<Stock>(Handle<Quote>(q));
    auto s2 = ext::make_shared<Stock>(Handle<Quote>());
    auto s3 = ext::make_shared<Stock>(Handle<Quote>());

    s3->registerWith(s2);
    s2->registerWith(s1);
    s1->registerWith(s3);

#ifdef QL_THROW_IN_CYCLES

    BOOST_CHECK_EXCEPTION(q->setValue(2.0), Error,
                          ExpectedErrorMessage("recursive notification loop detected"));

#else

    Flag f;
    f.registerWith(s3);
    q->setValue(2.0);

    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change");

#endif

}

test_suite* LazyObjectTest::suite() {
    auto* suite = BOOST_TEST_SUITE("LazyObject tests");
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testDiscardingNotifications));
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testDiscardingNotificationsByDefault));
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testForwardingNotificationsByDefault));
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testForwardingNotifications));
    suite->add(QUANTLIB_TEST_CASE(&LazyObjectTest::testNotificationLoop));
    return suite;
}
