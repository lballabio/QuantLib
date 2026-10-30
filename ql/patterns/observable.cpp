/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2013 Chris Higgs
Copyright (C) 2015 Klaus Spanderen
Copyright (C) 2026 Kyrylo Protsenko

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


#include <ql/patterns/observable.hpp>

#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

namespace QuantLib {

    void ObservableSettings::enableUpdates() {
        updatesEnabled_  = true;
        updatesDeferred_ = false;

        // if there are outstanding deferred updates, do the notification
        if (!deferredObservers_.empty()) {
            bool successful = true;
            std::string errMsg;
            runningDeferredUpdates_ = true;

            for (const auto& [deferredObserver, isValid] : deferredObservers_) {
                if (!isValid)
                    continue;

                try {
                    deferredObserver->update();
                } catch (std::exception& e) {
                    successful = false;
                    errMsg = e.what();
                } catch (...) {
                    successful = false;
                }
            }

            deferredObservers_.clear();
            runningDeferredUpdates_ = false;

            QL_ENSURE(successful,
                  "could not notify one or more observers: " << errMsg);
        }

    }


    void Observable::notifyObservers() {
        if (!ObservableSettings::instance().updatesEnabled()) {
            // if updates are only deferred, flag this for later notification
            // these are held centrally by the settings singleton
            ObservableSettings::instance().registerDeferredObservers(observers_);
        } else if (!observers_.empty()) {
            bool successful = true;
            std::string errMsg;
            for (auto* observer : observers_) {
                try {
                    observer->update();
                } catch (std::exception& e) {
                    // quite a dilemma. If we don't catch the exception,
                    // other observers will not receive the notification
                    // and might be left in an incorrect state. If we do
                    // catch it and continue the loop (as we do here) we
                    // lose the exception. The least evil might be to try
                    // and notify all observers, while raising an
                    // exception if something bad happened.
                    successful = false;
                    errMsg = e.what();
                } catch (...) {
                    successful = false;
                }
            }
            QL_ENSURE(successful,
                  "could not notify one or more observers: " << errMsg);
        }
    }

}

#else

#include <boost/signals2/signal_type.hpp>
#include <map>

namespace QuantLib {

    namespace detail {

        class Signal {
          public:
            typedef boost::signals2::signal_type<
                void(),
                boost::signals2::keywords::mutex_type<std::recursive_mutex> >
                ::type signal_type;

            /* Keeping each connection under QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN 
			   avoids searching all entries when an observer unregisters (common for coupons in rate helpers).
			   This speeds up cleanup for observables materially. */
            boost::signals2::connection connect(const signal_type::slot_type& slot) {
                return sig_.connect(slot);
            }

            void store(const void* observer, const boost::signals2::connection& c) {
                connections_[observer] = c;
            }

            boost::signals2::connection release(const void* observer) {
                auto i = connections_.find(observer);
                if (i == connections_.end())
                    return {};
                boost::signals2::connection c = i->second;
                connections_.erase(i);
                return c;
            }

            void operator()() const {
                sig_();
            }
          private:
            signal_type sig_;
            std::map<const void*, boost::signals2::connection> connections_;
        };

        template <class T>
        class ProxyUpdater {
            T* proxy_;
          public:
            explicit ProxyUpdater(const ext::shared_ptr<T>& observerProxy)
            : proxy_(observerProxy.get()) {}

            void operator()() const {
                proxy_->update();
            }
        };

    }

    void Observable::registerObserver(const ext::shared_ptr<Observer::Proxy>& observerProxy) {
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            // an observer registered twice is notified once, as in the
            // single-threaded implementation
            if (!observers_.insert(observerProxy).second)
                return;
        }

        detail::Signal::signal_type::slot_type slot {detail::ProxyUpdater<Observer::Proxy>(observerProxy)};
        #if defined(QL_USE_STD_SHARED_PTR)
        boost::signals2::connection connection = sig_->connect(slot.track_foreign(observerProxy));
        #else
        boost::signals2::connection connection = sig_->connect(slot.track(observerProxy));
        #endif

        std::lock_guard<std::recursive_mutex> lock(mutex_);
        sig_->store(observerProxy.get(), connection);
    }

    void Observable::unregisterObserver(const ext::shared_ptr<Observer::Proxy>& observerProxy,
                                        bool disconnect) {
        boost::signals2::connection connection;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            observers_.erase(observerProxy);
            connection = sig_->release(observerProxy.get());
        }

        if (ObservableSettings::instance().updatesDeferred()) {
            std::lock_guard<std::mutex> sLock(ObservableSettings::instance().mutex_);
            if (ObservableSettings::instance().updatesDeferred())
                ObservableSettings::instance().unregisterDeferredObserver(observerProxy);
        }

        // without disconnect the slot expires with the tracked proxy
        if (disconnect) {
            connection.disconnect();
        }
    }

    void Observable::notifyObservers() {
        if (ObservableSettings::instance().updatesEnabled()) {
            sig_->operator()();
        }
        else {
            bool updatesEnabled = false;
            {
                std::lock_guard<std::mutex> sLock(ObservableSettings::instance().mutex_);
                updatesEnabled = ObservableSettings::instance().updatesEnabled();

                if (ObservableSettings::instance().updatesDeferred()) {
                    std::lock_guard<std::recursive_mutex> lock(mutex_);
                    ObservableSettings::instance().registerDeferredObservers(observers_);
                }
            }

            if (updatesEnabled)
                sig_->operator()();
        }
    }

    Observable::Observable()
    : sig_(new detail::Signal()) { }

    Observable::Observable(const Observable&)
    : sig_(new detail::Signal()) {
        // the observer set is not copied; no observer asked to
        // register with this object
    }

}

#endif
