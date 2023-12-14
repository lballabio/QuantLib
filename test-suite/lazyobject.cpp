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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/stock.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using ext::shared_ptr;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(LazyObjectTests)

namespace {

    class TearDown { // NOLINT(cppcoreguidelines-special-member-functions)
        bool alwaysForward;
      public:
        TearDown() : alwaysForward(LazyObject::Defaults::instance().forwardsAllNotifications()) {}
        ~TearDown() {
            if (alwaysForward)
                LazyObject::Defaults::instance().alwaysForwardNotifications();
            else
                LazyObject::Defaults::instance().forwardFirstNotificationOnly();
        }
    };

}

BOOST_AUTO_TEST_CASE(testDiscardingNotifications) {

    BOOST_TEST_MESSAGE("Testing that lazy objects can discard notifications after the first against default...");

    TearDown teardown;

    LazyObject::Defaults::instance().alwaysForwardNotifications();

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

BOOST_AUTO_TEST_CASE(testDiscardingNotificationsByDefault) {

    BOOST_TEST_MESSAGE("Testing that lazy objects can discard notifications after the first by default...");

    TearDown teardown;

    LazyObject::Defaults::instance().forwardFirstNotificationOnly();

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

BOOST_AUTO_TEST_CASE(testForwardingNotificationsByDefault) {

    BOOST_TEST_MESSAGE("Testing that lazy objects can forward all notifications by default...");

    TearDown teardown;

    LazyObject::Defaults::instance().alwaysForwardNotifications();

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

BOOST_AUTO_TEST_CASE(testForwardingNotifications) {

    BOOST_TEST_MESSAGE("Testing that lazy objects can forward all notifications against default...");

    TearDown teardown;

    LazyObject::Defaults::instance().forwardFirstNotificationOnly();

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

BOOST_AUTO_TEST_CASE(testNotificationLoop) {

    BOOST_TEST_MESSAGE("Testing that lazy objects manage recursive notifications...");

    TearDown teardown;

    LazyObject::Defaults::instance().alwaysForwardNotifications();

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

    // We have produced a ring of dependencies which we break here
    // see https://github.com/lballabio/QuantLib/issues/1725
    s1->unregisterWithAll();
    s2->unregisterWithAll();
    s3->unregisterWithAll();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
