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

#include "observable.hpp"
#include "utilities.hpp"
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionlet.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class UpdateCounter : public Observer {
      public:
        UpdateCounter() : counter_(0) {}
        void update() {
            ++counter_;
        }
        Size counter() { return counter_; }
      private:
        Size counter_;
    };

    class RestoreUpdates {
      public:
        ~RestoreUpdates() {
            ObservableSettings::instance().enableUpdates();
        }
    };

}

void ObservableTest::testObservableSettings() {

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

#include <boost/atomic.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <list>

namespace {

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
        boost::atomic<int> counter_;
        static boost::atomic<int> instanceCounter_;
    };

    boost::atomic<int> MTUpdateCounter::instanceCounter_(0);

    class GarbageCollector {
      public:
        GarbageCollector() : terminate_(false) { }

        void addObj(const ext::shared_ptr<MTUpdateCounter>& updateCounter) {
            boost::lock_guard<boost::mutex> lock(mutex_);
            objList.push_back(updateCounter);
        }

        void run() {
            while(!terminate_) {
                Size objListSize;
                {
                    boost::lock_guard<boost::mutex> lock(mutex_);
                    objListSize = objList.size();
                }

                if (objListSize > 20) {
                    // trigger gc
                    while (objListSize > 0) {
                        boost::lock_guard<boost::mutex> lock(mutex_);
                        objList.pop_front();
                        objListSize = objList.size();
                    }
                }

                boost::this_thread::sleep(boost::posix_time::milliseconds(2));
            }
            objList.clear();
        }

        void terminate() {
            terminate_ = true;
        }
      private:
        boost::mutex mutex_;
        boost::atomic<bool> terminate_;

        std::list<ext::shared_ptr<MTUpdateCounter> > objList;
    };
}

void ObservableTest::testAsyncGarbagCollector() {

    BOOST_TEST_MESSAGE("Testing observer pattern with an asynchronous "
                       "garbage collector (JVM/.NET use case)...");

    // This test core dumps if used with the ordinary implementation
    // of the observer pattern (comparable situation
    // in JVM or .NET eco systems).

    const ext::shared_ptr<SimpleQuote> quote(new SimpleQuote(-1.0));

    GarbageCollector gc;
    boost::thread workerThread(&GarbageCollector::run, &gc);

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


void ObservableTest::testMultiThreadingGlobalSettings() {
	BOOST_TEST_MESSAGE("Testing observer global settings in a "
		               "multithreading environment...");
	
	const ext::shared_ptr<SimpleQuote> quote(new SimpleQuote(-1.0));

    ObservableSettings::instance().disableUpdates(true);

    GarbageCollector gc;
    boost::thread workerThread(&GarbageCollector::run, &gc);

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

void ObservableTest::testDeepUpdate() {

    SavedSettings backup;
    RestoreUpdates guard;

    Date refDate = Settings::instance().evaluationDate();

    ObservableSettings::instance().disableUpdates(true);

    Handle<YieldTermStructure> yts(
        ext::make_shared<FlatForward>(0, NullCalendar(), 0.02, Actual365Fixed()));
    ext::shared_ptr<IborIndex> ibor = ext::make_shared<Euribor>(3 * Months, yts);
    std::vector<Real> strikes;
    std::vector<Date> dates;
    std::vector<std::vector<Handle<Quote> > > quotes;
    strikes.push_back(0.01);
    strikes.push_back(0.02);
    dates.push_back(refDate + 90);
    dates.push_back(refDate + 180);
    ext::shared_ptr<SimpleQuote> q = ext::make_shared<SimpleQuote>(0.20);
    quotes.push_back(std::vector<Handle<Quote> >(2, Handle<Quote>(q)));
    quotes.push_back(std::vector<Handle<Quote> >(2, Handle<Quote>(q)));

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

    BOOST_CHECK_CLOSE(v1, 0.2, 1E-10);
    BOOST_CHECK_CLOSE(v2, 0.2, 1E-10);
    BOOST_CHECK_CLOSE(v3, 0.2, 1E-10);
    BOOST_CHECK_CLOSE(v4, 0.21, 1E-10);
}

test_suite* ObservableTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Observer tests");

    suite->add(QUANTLIB_TEST_CASE(&ObservableTest::testObservableSettings));

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
    suite->add(QUANTLIB_TEST_CASE(&ObservableTest::testAsyncGarbagCollector));
    suite->add(QUANTLIB_TEST_CASE(
        &ObservableTest::testMultiThreadingGlobalSettings));
#endif

    suite->add(QUANTLIB_TEST_CASE(&ObservableTest::testDeepUpdate));

    return suite;
}

