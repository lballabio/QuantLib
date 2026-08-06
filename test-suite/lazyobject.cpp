/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 StatPro Italia srl
 Copyright (C) 2026 Musawer Ahmad Saqif

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
#include <future>
#include <thread>
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

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

class BlockingLazyObject : public LazyObject {
  public:
    explicit BlockingLazyObject(bool failFirst = false) : failFirst_(failFirst) {}

    Real value() const {
        calculate();
        return value_;
    }

    void waitUntilCalculationStarts() const {
        while (!calculationStarted_.load(std::memory_order_acquire))
            std::this_thread::yield();
    }

    void releaseCalculation() const {
        calculationMayFinish_.store(true, std::memory_order_release);
    }

    Size calculationCount() const {
        return calculationCount_.load(std::memory_order_relaxed);
    }

  protected:
    void performCalculations() const override {
        const Size count = calculationCount_.fetch_add(1, std::memory_order_relaxed) + 1;
        if (count == 1) {
            calculationStarted_.store(true, std::memory_order_release);
            while (!calculationMayFinish_.load(std::memory_order_acquire))
                std::this_thread::yield();
            if (failFirst_)
                QL_FAIL("intentional failure");
        }
        value_ = 42.0;
    }

  private:
    bool failFirst_;
    mutable std::atomic<bool> calculationStarted_{false};
    mutable std::atomic<bool> calculationMayFinish_{false};
    mutable std::atomic<Size> calculationCount_{0};
    mutable Real value_ = Null<Real>();
};

BOOST_AUTO_TEST_CASE(testConcurrentCalculation) {
    BOOST_TEST_MESSAGE("Testing concurrent calculation of a lazy object...");

    BlockingLazyObject object;

    auto first = std::async(std::launch::async, [&object] { return object.value(); });
    object.waitUntilCalculationStarts();

    auto second = std::async(std::launch::async, [&object] { return object.value(); });
    const auto status = second.wait_for(std::chrono::milliseconds(20));
    object.releaseCalculation();

    BOOST_CHECK(status == std::future_status::timeout);
    BOOST_CHECK_EQUAL(first.get(), 42.0);
    BOOST_CHECK_EQUAL(second.get(), 42.0);
    BOOST_CHECK_EQUAL(object.calculationCount(), 1);
}

BOOST_AUTO_TEST_CASE(testConcurrentCalculationAfterFailure) {
    BOOST_TEST_MESSAGE("Testing concurrent calculation after a failure...");

    BlockingLazyObject object(true);

    auto first = std::async(std::launch::async, [&object] {
        try {
            object.value();
            return false;
        } catch (const Error&) {
            return true;
        }
    });
    object.waitUntilCalculationStarts();

    auto second = std::async(std::launch::async, [&object] { return object.value(); });
    const auto status = second.wait_for(std::chrono::milliseconds(20));
    object.releaseCalculation();

    BOOST_CHECK(status == std::future_status::timeout);
    BOOST_CHECK(first.get());
    BOOST_CHECK_EQUAL(second.get(), 42.0);
    BOOST_CHECK_EQUAL(object.calculationCount(), 2);
}

BOOST_AUTO_TEST_CASE(testConcurrentExpiredInstrumentCalculation) {
    BOOST_TEST_MESSAGE("Testing concurrent calculation of an expired instrument...");

    class ExpiredInstrument : public Instrument {
      public:
        bool isExpired() const override {
            return true;
        }

        void waitUntilCalculationStarts() const {
            while (!calculationStarted_.load(std::memory_order_acquire))
                std::this_thread::yield();
        }

        void releaseCalculation() const {
            calculationMayFinish_.store(true, std::memory_order_release);
        }

        Size calculationCount() const {
            return calculationCount_.load(std::memory_order_relaxed);
        }

      protected:
        void setupExpired() const override {
            const Size count = calculationCount_.fetch_add(1, std::memory_order_relaxed) + 1;
            if (count == 1) {
                calculationStarted_.store(true, std::memory_order_release);
                while (!calculationMayFinish_.load(std::memory_order_acquire))
                    std::this_thread::yield();
            }
            Instrument::setupExpired();
        }

      private:
        mutable std::atomic<bool> calculationStarted_{false};
        mutable std::atomic<bool> calculationMayFinish_{false};
        mutable std::atomic<Size> calculationCount_{0};
    } object;

    auto first = std::async(std::launch::async, [&object] { return object.NPV(); });
    object.waitUntilCalculationStarts();

    auto second = std::async(std::launch::async, [&object] { return object.NPV(); });
    const auto status = second.wait_for(std::chrono::milliseconds(20));
    object.releaseCalculation();

    BOOST_CHECK(status == std::future_status::timeout);
    BOOST_CHECK_EQUAL(first.get(), 0.0);
    BOOST_CHECK_EQUAL(second.get(), 0.0);
    BOOST_CHECK_EQUAL(object.calculationCount(), 1);
}

BOOST_AUTO_TEST_CASE(testRecursiveCalculation) {
    BOOST_TEST_MESSAGE("Testing recursive calculation of a lazy object...");

    class RecursiveLazyObject : public LazyObject {
      public:
        Real value() const {
            calculate();
            return value_;
        }

        Size calculationCount() const {
            return calculationCount_;
        }

      protected:
        void performCalculations() const override {
            ++calculationCount_;
            calculate();
            value_ = 42.0;
        }

      private:
        mutable Size calculationCount_ = 0;
        mutable Real value_ = Null<Real>();
    };

    RecursiveLazyObject object;
    BOOST_CHECK_EQUAL(object.value(), 42.0);
    BOOST_CHECK_EQUAL(object.calculationCount(), 1);
}

BOOST_AUTO_TEST_CASE(testCopyingWithThreadSafeCalculations) {
    BOOST_TEST_MESSAGE("Testing copies of lazy objects with thread-safe calculations...");

    class CopyableLazyObject : public LazyObject {
      public:
        explicit CopyableLazyObject(Real value = 0.0) : input_(value) {}

        Real value() const {
            calculate();
            return result_;
        }

      protected:
        void performCalculations() const override {
            result_ = input_;
        }

      private:
        Real input_;
        mutable Real result_ = Null<Real>();
    };

    CopyableLazyObject original(42.0);
    BOOST_CHECK_EQUAL(original.value(), 42.0);

    CopyableLazyObject copied(original);
    BOOST_CHECK_EQUAL(copied.value(), 42.0);

    CopyableLazyObject assigned;
    assigned = original;
    BOOST_CHECK_EQUAL(assigned.value(), 42.0);
}

#endif

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
