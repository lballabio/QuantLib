/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/stock.hpp>
#include <ql/quotes/simplequote.hpp>
#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;
using ext::shared_ptr;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(LazyObjectTests)

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

namespace {

    // calls back into itself while calculating, as curves do when bootstrapping
    class ReentrantLazyObject : public LazyObject {
      public:
        Real result() const {
            calculate();
            return result_;
        }
        Size calculations() const { return calculations_; }

      private:
        void performCalculations() const override {
            ++calculations_;
            result_ = 1.0;
            result_ = result() + 1.0;
        }
        mutable Real result_ = 0.0;
        mutable Size calculations_ = 0;
    };

}

BOOST_AUTO_TEST_CASE(testReentrantCalculation) {

    BOOST_TEST_MESSAGE(
        "Testing that lazy objects can call back into themselves while calculating...");

    ReentrantLazyObject object;

    BOOST_CHECK_EQUAL(object.result(), 2.0);
    BOOST_CHECK_EQUAL(object.result(), 2.0);
    BOOST_CHECK_EQUAL(object.calculations(), 1);
}

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

namespace {

    // takes its time to calculate, so that concurrent calls overlap
    class SlowLazyObject : public LazyObject {
      public:
        Real result() const {
            calculate();
            return result_;
        }
        int calculations() const { return calculations_; }
        bool overlapped() const { return overlapped_; }

      private:
        void performCalculations() const override {
            if (running_.exchange(true))
                overlapped_ = true;
            ++calculations_;
            result_ = 0.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            result_ = 42.0;
            running_ = false;
        }
        mutable Real result_ = 0.0;
        mutable std::atomic<int> calculations_{0};
        mutable std::atomic<bool> running_{false}, overlapped_{false};
    };

}

BOOST_AUTO_TEST_CASE(testConcurrentCalculation) {

    BOOST_TEST_MESSAGE("Testing that concurrent calculations of a lazy object are serialized...");

    SlowLazyObject object;

    const Size n = 8;
    std::vector<Real> results(n, 0.0);
    std::vector<std::thread> threads;
    threads.reserve(n);
    for (Size i = 0; i < n; ++i)
        threads.emplace_back([&object, &results, i] { results[i] = object.result(); });
    for (auto& t : threads)
        t.join();

    if (object.calculations() != 1)
        BOOST_ERROR("calculations were performed " << object.calculations() << " times");
    if (object.overlapped())
        BOOST_ERROR("calculations were performed concurrently");
    for (Size i = 0; i < n; ++i) {
        if (results[i] != 42.0)
            BOOST_ERROR("thread " << i << " read " << results[i]
                                  << " while the calculation was still running");
    }
}

#endif

BOOST_AUTO_TEST_CASE(testNotificationAfterFailedCalculation) {

    BOOST_TEST_MESSAGE("Testing that lazy objects forward notifications after a failed calculation...");

    TearDown teardown;

    LazyObject::Defaults::instance().forwardFirstNotificationOnly();

    // a lazy object whose performCalculations() can be made to fail
    class Failing : public LazyObject {
        mutable bool fail_ = false;
      public:
        void failOnCalculation(bool b) { fail_ = b; }
        void doCalculate() const { calculate(); }
        void performCalculations() const override {
            if (fail_)
                QL_FAIL("intentional failure");
        }
    };

    auto q = ext::make_shared<SimpleQuote>(0.0);
    auto s = ext::make_shared<Failing>();
    s->registerWith(q);

    Flag f;
    f.registerWith(s);

    // successful calculation, then change => observer should be notified
    s->doCalculate();
    q->setValue(1.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change after successful calculation");

    f.lower();

    // failed calculation
    s->failOnCalculation(true);
    BOOST_CHECK_EXCEPTION(s->doCalculate(), Error,
                          ExpectedErrorMessage("intentional failure"));

    if (f.isUp())
        BOOST_FAIL("Observer was notified by failed calculation itself");

    // fix the object
    s->failOnCalculation(false);

    // change input => observer should be notified despite the prior failure
    q->setValue(2.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change after failed calculation");

    f.lower();

    // verify it can actually recalculate now
    BOOST_CHECK_NO_THROW(s->doCalculate());

    if (f.isUp())
        BOOST_FAIL("Observer was notified by successful recalculation itself");

    // verify the "forward first only" contract is preserved:
    // after recalculation, one notification should be forwarded...
    q->setValue(3.0);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of change after recovery");

    f.lower();

    // ...but a second change without recalculation should be discarded
    q->setValue(4.0);
    if (f.isUp())
        BOOST_FAIL("Observer was notified of second change without recalculation");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
