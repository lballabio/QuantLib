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
#include <ql/errors.hpp>

#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

namespace QuantLib {

    void ObservableSettings::disableUpdates(bool deferred) {
        updatesEnabled_ = false;
        updatesDeferred_ = deferred;
    }

    void ObservableSettings::enableUpdates() {
        updatesEnabled_  = true;
        updatesDeferred_ = false;

        // if there are outstanding deferred updates, do the notification
        if (!deferredObservers_.empty()) {
            bool successful = true;
            std::string errMsg;

            for (auto* deferredObserver : deferredObservers_) {
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

            QL_ENSURE(successful,
                  "could not notify one or more observers: " << errMsg);
        }
    }

    bool ObservableSettings::updatesEnabled() const { return updatesEnabled_; }
    bool ObservableSettings::updatesDeferred() const { return updatesDeferred_; }

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

    Observable::Observable() = default;

    void ObservableSettings::registerDeferredObservers(const Observable::set_type& observers) {
        if (updatesDeferred()) {
            deferredObservers_.insert(observers.begin(), observers.end());
        }
    }

    void ObservableSettings::unregisterDeferredObserver(Observer* o) {
        deferredObservers_.erase(o);
    }

    Observable::Observable(const Observable&) {
        // the observer set is not copied; no observer asked to
        // register with this object
    }

    /*! \warning notification is sent before the copy constructor has
                 a chance of actually change the data
                 members. Therefore, observers whose update() method
                 tries to use their observables will not see the
                 updated values. It is suggested that the update()
                 method just raise a flag in order to trigger
                 a later recalculation.
    */
    Observable& Observable::operator=(const Observable& o) {
        // as above, the observer set is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
    }

    std::pair<Observable::iterator, bool>
    Observable::registerObserver(Observer* o) {
        return observers_.insert(o);
    }

    Size Observable::unregisterObserver(Observer* o) {
        if (ObservableSettings::instance().updatesDeferred())
            ObservableSettings::instance().unregisterDeferredObserver(o);

        return observers_.erase(o);
    }


    Observer::Observer(const Observer& o)
    : observables_(o.observables_) {
        for (const auto& observable : observables_)
            observable->registerObserver(this);
    }

    Observer& Observer::operator=(const Observer& o) {
        for (const auto& observable : observables_)
            observable->unregisterObserver(this);
        observables_ = o.observables_;
        for (const auto& observable : observables_)
            observable->registerObserver(this);
        return *this;
    }

    Observer::~Observer() {
        for (const auto& observable : observables_)
            observable->unregisterObserver(this);
    }

    std::pair<Observer::iterator, bool>
    Observer::registerWith(const ext::shared_ptr<Observable>& h) {
        if (h != nullptr) {
            h->registerObserver(this);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
    }

    void Observer::registerWithObservables(const ext::shared_ptr<Observer>& o) {
        if (o != nullptr) {
            for (const auto& observable : o->observables_)
                registerWith(observable);
        }
    }

    Size Observer::unregisterWith(const ext::shared_ptr<Observable>& h) {
        if (h != nullptr)
            h->unregisterObserver(this);
        return observables_.erase(h);
    }

    void Observer::unregisterWithAll() {
        for (const auto& observable : observables_)
            observable->unregisterObserver(this);
        observables_.clear();
    }

    void Observer::deepUpdate() {
        update();
    }
}

#else

#include <boost/signals2/signal_type.hpp>

namespace QuantLib {

    namespace detail {

        class Signal {
          public:
            typedef boost::signals2::signal_type<
                void(),
                boost::signals2::keywords::mutex_type<std::recursive_mutex> >
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

        template <class T>
        class ProxyUpdater {
            T* proxy_;
          public:
            explicit ProxyUpdater(const ext::shared_ptr<T>& observerProxy)
            : proxy_(observerProxy.get()) {}

            void operator()() const {
                proxy_->update();
            }

            bool operator==(const ProxyUpdater<T>& other) const {
                return proxy_ == other.proxy_;
            }

            bool operator!=(const ProxyUpdater<T>& other) const {
                return proxy_ != other.proxy_;
            }
        };

    }

    void Observer::Proxy::update() const {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (active_) {
            // c++17 is required if used with std::shared_ptr<T>
            const ext::weak_ptr<Observer> o = observer_->weak_from_this();

            // check for empty weak reference
            // https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
            const ext::weak_ptr<Observer> empty;
            if (o.owner_before(empty) || empty.owner_before(o)) {
                const ext::shared_ptr<Observer> obs(o.lock());
                if (obs)
                    obs->update();
            } else {
                observer_->update();
            }
        }
    }

    void Observer::Proxy::deactivate() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        active_ = false;
    }


    void Observable::registerObserver(const ext::shared_ptr<Observer::Proxy>& observerProxy) {
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            observers_.insert(observerProxy);
        }

        detail::Signal::signal_type::slot_type slot {detail::ProxyUpdater<Observer::Proxy>(observerProxy)};
        #if defined(QL_USE_STD_SHARED_PTR)
        sig_->connect(slot.track_foreign(observerProxy));
        #else
        sig_->connect(slot.track(observerProxy));
        #endif
    }

    void Observable::unregisterObserver(const ext::shared_ptr<Observer::Proxy>& observerProxy,
                                        bool disconnect) {
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            observers_.erase(observerProxy);
        }

        if (ObservableSettings::instance().updatesDeferred()) {
            std::lock_guard<std::mutex> sLock(ObservableSettings::instance().mutex_);
            if (ObservableSettings::instance().updatesDeferred()) {
                ObservableSettings::instance().unregisterDeferredObserver(observerProxy);
            }
        }

        if (disconnect) {
            sig_->disconnect(detail::ProxyUpdater<Observer::Proxy>(observerProxy));
        }
    }

    void Observable::notifyObservers() {
        if (ObservableSettings::instance().updatesEnabled()) {
            return (*sig_)();
        }

        std::lock_guard<std::mutex> sLock(ObservableSettings::instance().mutex_);
        if (ObservableSettings::instance().updatesEnabled()) {
            return (*sig_)();
        }
        else if (ObservableSettings::instance().updatesDeferred()) {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            // if updates are only deferred, flag this for later notification
            // these are held centrally by the settings singleton
            ObservableSettings::instance().registerDeferredObservers(observers_);
        }
    }

    Observable::Observable()
    : sig_(new detail::Signal()) { }

    Observable::Observable(const Observable&)
    : sig_(new detail::Signal()) {
        // the observer set is not copied; no observer asked to
        // register with this object
    }

    void ObservableSettings::registerDeferredObservers(const Observable::set_type& observers) {
        deferredObservers_.insert(observers.begin(), observers.end());
    }

    void ObservableSettings::unregisterDeferredObserver(
        const ext::shared_ptr<Observer::Proxy>& o) {
        deferredObservers_.erase(o);
    }

    void ObservableSettings::enableUpdates() {
        std::lock_guard<std::mutex> lock(mutex_);

        // if there are outstanding deferred updates, do the notification
        updatesType_ = UpdatesEnabled;

        if (deferredObservers_.size()) {
            bool successful = true;
            std::string errMsg;

            for (auto i=deferredObservers_.begin();
                i!=deferredObservers_.end(); ++i) {
                try {
                    const ext::shared_ptr<Observer::Proxy> proxy = i->lock();
                    if (proxy)
                        proxy->update();
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

    /*! \warning notification is sent before the copy constructor has
             a chance of actually change the data
             members. Therefore, observers whose update() method
             tries to use their observables will not see the
             updated values. It is suggested that the update()
             method just raise a flag in order to trigger
            a later recalculation.
    */
    Observable& Observable::operator=(const Observable& o) {
        // as above, the observer set is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
    }

    Observer::Observer(const Observer& o) {
        proxy_.reset(new Proxy(this));

        {
             std::lock_guard<std::recursive_mutex> lock(o.mutex_);
             observables_ = o.observables_;
        }

        for (const auto& observable : observables_)
            observable->registerObserver(proxy_);
    }

    Observer& Observer::operator=(const Observer& o) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!proxy_) {
            proxy_.reset(new Proxy(this));
        }

        for (const auto& observable : observables_)
            observable->unregisterObserver(proxy_, true);

        {
            std::lock_guard<std::recursive_mutex> lock(o.mutex_);
            observables_ = o.observables_;
        }
        for (const auto& observable : observables_)
            observable->registerObserver(proxy_);

        return *this;
    }

    Observer::~Observer() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (proxy_)
            proxy_->deactivate();

        for (const auto& observable : observables_)
            observable->unregisterObserver(proxy_, false);
    }

    std::pair<Observer::iterator, bool>
    Observer::registerWith(const ext::shared_ptr<Observable>& h) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!proxy_) {
            proxy_.reset(new Proxy(this));
        }

        if (h) {
            h->registerObserver(proxy_);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
    }

    void
    Observer::registerWithObservables(const ext::shared_ptr<Observer>& o) {
        if (o) {
            std::lock_guard<std::recursive_mutex> lock(o->mutex_);

            for (const auto& observable : o->observables_)
                registerWith(observable);
        }
    }

    inline
    Size Observer::unregisterWith(const ext::shared_ptr<Observable>& h) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (h && proxy_)  {
            h->unregisterObserver(proxy_, true);
        }

        return observables_.erase(h);
    }

    void Observer::unregisterWithAll() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        for (const auto& observable : observables_)
            observable->unregisterObserver(proxy_, true);

        observables_.clear();
    }

    void Observer::deepUpdate() {
        update();
    }

}

#endif
