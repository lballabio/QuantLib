/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2013 Chris Higgs
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

            for (iterator i=deferredObservers_.begin();
                i!=deferredObservers_.end(); ++i) {
                try {
                    (*i)->update();
                } catch (std::exception& e) {
                    successful = false;
                    errMsg = e.what();
                } catch (...) {
                    successful = false;
                }
            }

            deferredObservers_.clear();

            QL_ENSURE(successful,
                  "could not notify one or more observers: " << errMsg);
        }
    }


    void Observable::notifyObservers() {
        if (!settings_.updatesEnabled()) {
            // if updates are only deferred, flag this for later notification
            // these are held centrally by the settings singleton
            settings_.registerDeferredObservers(observers_);
        } else if (!observers_.empty()) {
            bool successful = true;
            std::string errMsg;
            for (iterator i=observers_.begin(); i!=observers_.end(); ++i) {
                try {
                    (*i)->update();
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

#include <ql/functional.hpp>

#if defined(QL_USE_STD_FUNCTION)
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/bind/bind.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif
#endif

#include <boost/signals2/signal_type.hpp>

namespace QuantLib {

    namespace detail {

        class Signal {
          public:
            typedef boost::signals2::signal_type<
                void(),
                boost::signals2::keywords::mutex_type<boost::recursive_mutex> >
                ::type signal_type;

            void connect(const signal_type::slot_type& slot) {
                sig_.connect(slot);
            }

            template <class T>
            void disconnect(const T& slot) {
                sig_.disconnect(slot);
            }

            void operator()() const {
                sig_();
            }
          private:
            signal_type sig_;
        };

    }

    void Observable::registerObserver(
        const ext::shared_ptr<Observer::Proxy>& observerProxy) {
        {
            boost::lock_guard<boost::recursive_mutex> lock(mutex_);
            observers_.insert(observerProxy);
        }

        detail::Signal::signal_type::slot_type slot(&Observer::Proxy::update,
                                    observerProxy.get());
        #if defined(QL_USE_STD_SHARED_PTR)
        sig_->connect(slot.track_foreign(observerProxy));
        #else
        sig_->connect(slot.track(observerProxy));
        #endif
    }

    void Observable::unregisterObserver(
        const ext::shared_ptr<Observer::Proxy>& observerProxy) {
        {
            boost::lock_guard<boost::recursive_mutex> lock(mutex_);
            observers_.erase(observerProxy);
        }

        if (settings_.updatesDeferred()) {
            boost::lock_guard<boost::mutex> sLock(settings_.mutex_);
            if (settings_.updatesDeferred()) {
                settings_.unregisterDeferredObserver(observerProxy);
            }
        }

        // signals2 needs boost::bind, std::bind does not work
        sig_->disconnect(boost::bind(&Observer::Proxy::update,
                             observerProxy.get()));
    }

    void Observable::notifyObservers() {
        if (settings_.updatesEnabled()) {
            return (*sig_)();
        }

        boost::lock_guard<boost::mutex> sLock(settings_.mutex_);
        if (settings_.updatesEnabled()) {
            return (*sig_)();
        }
        else if (settings_.updatesDeferred()) {
            boost::lock_guard<boost::recursive_mutex> lock(mutex_);
            // if updates are only deferred, flag this for later notification
            // these are held centrally by the settings singleton
            settings_.registerDeferredObservers(observers_);
        }
    }

    Observable::Observable()
    : sig_(new detail::Signal()),
      settings_(ObservableSettings::instance()) { }

    Observable::Observable(const Observable&)
    : sig_(new detail::Signal()),
      settings_(ObservableSettings::instance()) {
        // the observer set is not copied; no observer asked to
        // register with this object
    }

}

#endif
