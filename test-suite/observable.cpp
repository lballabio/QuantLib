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
#include <boost/make_shared.hpp>
#include <iostream>
#include <ql/patterns/observable.hpp>
#include <ql/quotes/simplequote.hpp>

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

    class ExtQuote : public SimpleQuote, public Observer {
       public:
        ExtQuote(Real quote) : SimpleQuote(quote), counter_(0) {}
        void update() {
            ++counter_;
            qv_ += this->value();
        }
        Size getqv() { return qv_; }
        Size counter() { return counter_; }

       private:
        Size counter_;
        Size qv_;
    };
}

void ObservableTest::testObservableSettings() {

    BOOST_TEST_MESSAGE("Testing observable settings...");

    const boost::shared_ptr<SimpleQuote> quote(new SimpleQuote(100.0));
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

   ExtQuote updateCounter3(0);
   ObservableSettings::instance().enableUpdates();
   std::vector<boost::shared_ptr<SimpleQuote> > quotes;
   int quoteNum = 100;
   for (int i = 0; i < quoteNum; ++i) {
       quotes.push_back(boost::make_shared<SimpleQuote>(1));
       updateCounter3.registerWith(quotes.back());
   }

   std::cout << "\n1. Number of updates: " << updateCounter3.counter();

   for (int i = 0; i < quoteNum; ++i) {
       quotes[i]->setValue(2);
   }

   std::cout << "\n2. Number of updates: " << updateCounter3.counter();

   ObservableSettings::instance().turnOffNotifications();

   for (int i = 0; i < quoteNum; ++i) {
       quotes[i]->setValue(3);
   }

   std::cout << "\n3. Number of updates: " << updateCounter3.counter();

   ObservableSettings::instance().forceAllObserverUpdates();
   std::cout << "\n4. Number of updates: " << updateCounter3.counter();

   ObservableSettings::instance().turnOnNotifications();

   for (int i = 0; i < quoteNum; ++i) {
       quotes[i]->setValue(4);
   }

   std::cout << "\n5. Number of updates: " << updateCounter3.counter() << ", "
             << updateCounter3.getqv();
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

        void addObj(const boost::shared_ptr<MTUpdateCounter>& updateCounter) {
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

        std::list<boost::shared_ptr<MTUpdateCounter> > objList;
    };
}

void ObservableTest::testAsyncGarbagCollector() {

    BOOST_TEST_MESSAGE("Testing observer pattern with an asynchronous "
                       "garbage collector (JVM/.NET use case)...");

    // This test core dumps if used with the ordinary implementation
    // of the observer pattern (comparable situation
    // in JVM or .NET eco systems).

    const boost::shared_ptr<SimpleQuote> quote(new SimpleQuote(-1.0));

    GarbageCollector gc;
    boost::thread workerThread(&GarbageCollector::run, &gc);

    for (Size i=0; i < 10000; ++i) {
        const boost::shared_ptr<MTUpdateCounter> observer(new MTUpdateCounter);
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
	
	const boost::shared_ptr<SimpleQuote> quote(new SimpleQuote(-1.0));

    ObservableSettings::instance().disableUpdates(true);

    GarbageCollector gc;
    boost::thread workerThread(&GarbageCollector::run, &gc);

    typedef std::list<boost::shared_ptr<MTUpdateCounter> > local_list_type;
    local_list_type localList;

    for (Size i=0; i < 4000; ++i) {
        const boost::shared_ptr<MTUpdateCounter> observer(new MTUpdateCounter);
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

    if (localList.size() != MTUpdateCounter::instanceCounter()) {
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



test_suite* ObservableTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Observer tests");

    suite->add(QUANTLIB_TEST_CASE(&ObservableTest::testObservableSettings));

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
    suite->add(QUANTLIB_TEST_CASE(&ObservableTest::testAsyncGarbagCollector));
    suite->add(QUANTLIB_TEST_CASE(
        &ObservableTest::testMultiThreadingGlobalSettings));
#endif

    return suite;
}

