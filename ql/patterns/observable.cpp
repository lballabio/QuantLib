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


#include <ql/qldefines.hpp>

#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

#include <ql/patterns/observable.hpp>
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
                sig_.operator()();
            }
          private:
            signal_type sig_;
        };
    }

    void Observable::registerObserver(
        const boost::shared_ptr<Observer::Proxy>& observerProxy) {
        {
            boost::lock_guard<boost::recursive_mutex> lock(mutex_);
            observers_.insert(observerProxy);
        }

        detail::Signal::signal_type::slot_type slot(&Observer::Proxy::update,
                                    observerProxy.get());
        sig_->connect(slot.track(observerProxy));
    }

    void Observable::unregisterObserver(
        const boost::shared_ptr<Observer::Proxy>& observerProxy) {
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

        sig_->disconnect(boost::bind(&Observer::Proxy::update,
                             observerProxy.get()));
    }

    void Observable::notifyObservers() {
        if (settings_.updatesEnabled()) {
            return sig_->operator()();
        }

        boost::lock_guard<boost::mutex> sLock(settings_.mutex_);
        if (settings_.updatesEnabled()) {
            return sig_->operator()();
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
