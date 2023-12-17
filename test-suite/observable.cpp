/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Klaus Spanderen

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
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionlet.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <chrono>
#include <thread>

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
#include <atomic>
#include <mutex>
#include <thread>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <list>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ObservableTests)

class UpdateCounter : public Observer {
  public:
    UpdateCounter() = default;
    void update() override { ++counter_; }
    Size counter() const { return counter_; }

  private:
    Size counter_ = 0;
};

class RestoreUpdates { // NOLINT(cppcoreguidelines-special-member-functions)
  public:
    ~RestoreUpdates() {
        ObservableSettings::instance().enableUpdates();
    }
};

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
class MTUpdateCounter : public Observer {
  public:
    MTUpdateCounter() : counter_(0) {
        ++instanceCounter_;
    }
    ~MTUpdateCounter() {
        --instanceCounter_;
    }
    void update() {
        ++counter_;
    }
    int counter() { return counter_; }
    static int instanceCounter() { return instanceCounter_; }

  private:
    std::atomic<int> counter_;
    static std::atomic<int> instanceCounter_;
};

std::atomic<int> MTUpdateCounter::instanceCounter_(0);

class GarbageCollector {
  public:
    GarbageCollector() : terminate_(false) { }

    void addObj(const ext::shared_ptr<MTUpdateCounter>& updateCounter) {
        std::lock_guard<std::mutex> lock(mutex_);
        objList.push_back(updateCounter);
    }

    void run() {
        while(!terminate_) {
            Size objListSize;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                objListSize = objList.size();
            }

            if (objListSize > 20) {
                // trigger gc
                while (objListSize > 0) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    objList.pop_front();
                    objListSize = objList.size();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        objList.clear();
    }

    void terminate() {
        terminate_ = true;
    }
  private:
    std::mutex mutex_;
    std::atomic<bool> terminate_;

    std::list<ext::shared_ptr<MTUpdateCounter> > objList;
};
#endif


BOOST_AUTO_TEST_CASE(testObservableSettings) {

    BOOST_TEST_MESSAGE("Testing observable settings...");

    const ext::shared_ptr<SimpleQuote> quote(new SimpleQuote(100.0));
    UpdateCounter updateCounter;

    updateCounter.registerWith(quote);
    if (updateCounter.counter() != 0) {
        BOOST_FAIL("update counter value is not zero");
    }

   quote->setValue(1.0);
   if (updateCounter.counter() != 1) {
       BOOST_FAIL("update counter value is not one");
   }

   ObservableSettings::instance().disableUpdates(false);
   quote->setValue(2.0);
   if (updateCounter.counter() != 1) {
       BOOST_FAIL("update counter value is not one");
   }
   ObservableSettings::instance().enableUpdates();
   if (updateCounter.counter() != 1) {
       BOOST_FAIL("update counter value is not one");
   }

   ObservableSettings::instance().disableUpdates(true);
   quote->setValue(3.0);
   if (updateCounter.counter() != 1) {
       BOOST_FAIL("update counter value is not one");
   }
   ObservableSettings::instance().enableUpdates();
   if (updateCounter.counter() != 2) {
       BOOST_FAIL("update counter value is not two");
   }

   UpdateCounter updateCounter2;
   updateCounter2.registerWith(quote);
   ObservableSettings::instance().disableUpdates(true);
   for (Size i=0; i < 10; ++i) {
       quote->setValue(Real(i));
   }
   if (updateCounter.counter() != 2) {
       BOOST_FAIL("update counter value is not two");
   }
   ObservableSettings::instance().enableUpdates();
   if (updateCounter.counter() != 3 || updateCounter2.counter() != 1) {
       BOOST_FAIL("update counter values are not correct");
   }
}


#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

BOOST_AUTO_TEST_CASE(testAsyncGarbagCollector) {

    BOOST_TEST_MESSAGE("Testing observer pattern with an asynchronous "
                       "garbage collector (JVM/.NET use case)...");

    // This test core dumps if used with the ordinary implementation
    // of the observer pattern (comparable situation
    // in JVM or .NET eco systems).

    const ext::shared_ptr<SimpleQuote> quote(new SimpleQuote(-1.0));

    GarbageCollector gc;
    std::thread workerThread(&GarbageCollector::run, &gc);

    for (Size i=0; i < 10000; ++i) {
        const ext::shared_ptr<MTUpdateCounter> observer(new MTUpdateCounter);
        observer->registerWith(quote);
        gc.addObj(observer);

        for (Size j=0; j < 10; ++j)
            quote->setValue(Real(j));
    }

    gc.terminate();
    workerThread.join();

    if (MTUpdateCounter::instanceCounter() != 0) {
        BOOST_FAIL("garbage collection does not work.");
    }
}

BOOST_AUTO_TEST_CASE(testMultiThreadingGlobalSettings) {
	BOOST_TEST_MESSAGE("Testing observer global settings in a "
		               "multithreading environment...");
	
	const ext::shared_ptr<SimpleQuote> quote(new SimpleQuote(-1.0));

    ObservableSettings::instance().disableUpdates(true);

    GarbageCollector gc;
    std::thread workerThread(&GarbageCollector::run, &gc);

    typedef std::list<ext::shared_ptr<MTUpdateCounter> > local_list_type;
    local_list_type localList;

    for (Size i=0; i < 4000; ++i) {
        const ext::shared_ptr<MTUpdateCounter> observer(new MTUpdateCounter);
        observer->registerWith(quote);

        if ((i%4) == 0) {
            localList.push_back(observer);
            for (Size j=0; j < 5; ++j)
                quote->setValue(Real(j));
        }
        gc.addObj(observer);
    }

    gc.terminate();
    workerThread.join();

    if (localList.size() != Size(MTUpdateCounter::instanceCounter())) {
        BOOST_FAIL("garbage collection does not work.");
    }

    for (local_list_type::iterator iter = localList.begin();
        iter != localList.end(); ++iter) {
        if ((*iter)->counter() != 0) {
            BOOST_FAIL("notification should have been blocked");
        }
    }

    ObservableSettings::instance().enableUpdates();

    for (local_list_type::iterator iter = localList.begin();
        iter != localList.end(); ++iter) {
        if ((*iter)->counter() != 1) {
            BOOST_FAIL("only one notification should have been sent");
        }
    }
}
#endif

BOOST_AUTO_TEST_CASE(testDeepUpdate) {
    BOOST_TEST_MESSAGE("Testing deep update of observers...");

    RestoreUpdates guard;

    Date refDate = Settings::instance().evaluationDate();

    ObservableSettings::instance().disableUpdates(true);

    Handle<YieldTermStructure> yts(
        ext::make_shared<FlatForward>(0, NullCalendar(), 0.02, Actual365Fixed()));
    ext::shared_ptr<IborIndex> ibor = ext::make_shared<Euribor>(3 * Months, yts);
    ext::shared_ptr<SimpleQuote> q = ext::make_shared<SimpleQuote>(0.20);
    std::vector<Real> strikes = {0.01, 0.02};
    std::vector<Date> dates = {refDate + 90, refDate + 180};
    std::vector<std::vector<Handle<Quote> > > quotes = {
        {Handle<Quote>(q), Handle<Quote>(q)},
        {Handle<Quote>(q), Handle<Quote>(q)}
    };

    ext::shared_ptr<StrippedOptionletAdapter> vol =
        ext::make_shared<StrippedOptionletAdapter>(ext::make_shared<StrippedOptionlet>(
            0, NullCalendar(), Unadjusted, ibor, dates, strikes, quotes, Actual365Fixed()));

    Real v1 = vol->volatility(refDate + 100, 0.01);
    q->setValue(0.21);
    Real v2 = vol->volatility(refDate + 100, 0.01);
    vol->update();
    Real v3 = vol->volatility(refDate + 100, 0.01);
    vol->deepUpdate();
    Real v4 = vol->volatility(refDate + 100, 0.01);

    QL_CHECK_CLOSE(v1, 0.2, 1E-10);
    QL_CHECK_CLOSE(v2, 0.2, 1E-10);
    QL_CHECK_CLOSE(v3, 0.2, 1E-10);
    QL_CHECK_CLOSE(v4, 0.21, 1E-10);
}


class DummyObserver : public Observer {
  public:
    DummyObserver() = default;
    void update() override {}
};


BOOST_AUTO_TEST_CASE(testEmptyObserverList) {
	BOOST_TEST_MESSAGE("Testing unregisterWith call on empty observer...");

    const ext::shared_ptr<DummyObserver> dummyObserver=ext::make_shared<DummyObserver>();
    dummyObserver->unregisterWith(ext::make_shared<SimpleQuote>(10.0));
}

BOOST_AUTO_TEST_CASE(testAddAndDeleteObserverDuringNotifyObservers) {
    BOOST_TEST_MESSAGE("Testing addition and deletion of observers during notifyObserver...");

    const ext::shared_ptr<MersenneTwisterUniformRng> rng
        = ext::make_shared<MersenneTwisterUniformRng>();

    const Size nrInitialObserver = 20;
    const Size nrDeleteDuringUpdate = 5;
    const Size nrAdditionalObserver = 100;
    const Size testRuns = 100;

    class TestSetup {
      public:
        explicit TestSetup(ext::shared_ptr<MersenneTwisterUniformRng> m)
        : rng(std::move(m)), observable(ext::make_shared<Observable>()) {}

        ext::shared_ptr<MersenneTwisterUniformRng> rng;
        ext::shared_ptr<Observable> observable;
        std::vector<ext::shared_ptr<Observer> > expected;
        std::vector<ext::shared_ptr<Observer> > additinalObservers;
    };

    class TestObserver: public Observer {
      public:
        explicit TestObserver(TestSetup* setup = nullptr) : setup_(setup) {}

        void update() override {
            ++updates_;

            if (setup_ != nullptr) {
                for (Size i=0; i < nrAdditionalObserver; ++i) {
                    const ext::shared_ptr<Observer> obs
                        = ext::make_shared<TestObserver>();

                    obs->registerWith(setup_->observable);
                    setup_->additinalObservers.push_back(obs);
                }

                for (Size i=0; i < nrDeleteDuringUpdate; ++i) {
                    const unsigned int j
                        = setup_->rng->nextInt32() % setup_->expected.size();

                    if (setup_->expected[j].get() != this)
                        setup_->expected.erase(setup_->expected.begin()+j);
                }
            }
        }

        Size getUpdates() const { return updates_; }

      private:
        TestSetup* const setup_;
        Size updates_ = 0;
    };

    for (Size t=0; t < testRuns; ++t) {
        const ext::shared_ptr<TestSetup> setup = ext::make_shared<TestSetup>(rng);

        for (Size i=0; i < nrInitialObserver; ++i) {
            const ext::shared_ptr<Observer> obs = 
                (i == nrInitialObserver/3 || i == nrInitialObserver/2)
                ? ext::make_shared<TestObserver>(setup.get())
                : ext::make_shared<TestObserver>();

            obs->registerWith(setup->observable);
            setup->expected.push_back(obs);
        }

        setup->observable->notifyObservers();

        for (const auto& obs : setup->expected)
            if (ext::dynamic_pointer_cast<TestObserver>(obs)->getUpdates() == 0) {
                BOOST_FAIL("missed observer update detected");
            }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
