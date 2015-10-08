/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
Copyright (C) 2011, 2012 Ferdinando Ametrano
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

/*! \file observable.hpp
    \brief observer/observable pattern
*/

#ifndef quantlib_observable_hpp
#define quantlib_observable_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
namespace QuantLib {

    class Observer;

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
      public:
        // constructors, assignment, destructor
        Observable() {}
        Observable(const Observable&);
        Observable& operator=(const Observable&);
        virtual ~Observable() {}
        /*! This method should be called at the end of non-const methods
            or when the programmer desires to notify any changes.
        */
        void notifyObservers();
      private:
        typedef boost::unordered_set<Observer*>::iterator iterator;
        std::pair<iterator, bool> registerObserver(Observer*);
        Size unregisterObserver(Observer*);
        boost::unordered_set<Observer*> observers_;
    };

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer {
      public:
        // constructors, assignment, destructor
        Observer() {}
        Observer(const Observer&);
        Observer& operator=(const Observer&);
        virtual ~Observer();
        // observer interface
        std::pair<boost::unordered_set<boost::shared_ptr<Observable> >::iterator, bool>
        registerWith(const boost::shared_ptr<Observable>&);
        /*! register with all observables of a given observer. Note
            that this does not include registering with the observer
            itself. */
        void registerWithObservables(const boost::shared_ptr<Observer>&);
        Size unregisterWith(const boost::shared_ptr<Observable>&);
        void unregisterWithAll();
        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        virtual void update() = 0;
      private:
        boost::unordered_set<boost::shared_ptr<Observable> > observables_;
        typedef boost::unordered_set<boost::shared_ptr<Observable> >::iterator iterator;
    };


    // inline definitions

    inline Observable::Observable(const Observable&) {
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
    inline Observable& Observable::operator=(const Observable& o) {
        // as above, the observer set is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
    }

    inline std::pair<boost::unordered_set<Observer*>::iterator, bool>
    Observable::registerObserver(Observer* o) {
        return observers_.insert(o);
    }

    inline Size Observable::unregisterObserver(Observer* o) {
        return observers_.erase(o);
    }

    inline void Observable::notifyObservers() {
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


    inline Observer::Observer(const Observer& o)
    : observables_(o.observables_) {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->registerObserver(this);
    }

    inline Observer& Observer::operator=(const Observer& o) {
        iterator i;
        for (i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
        observables_ = o.observables_;
        for (i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->registerObserver(this);
        return *this;
    }

    inline Observer::~Observer() {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
    }

    inline std::pair<boost::unordered_set<boost::shared_ptr<Observable> >::iterator, bool>
    Observer::registerWith(const boost::shared_ptr<Observable>& h) {
        if (h) {
            h->registerObserver(this);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
    }

    inline void
    Observer::registerWithObservables(const boost::shared_ptr<Observer> &o) {
        if (o) {
            iterator i;
            for (i = o->observables_.begin(); i != o->observables_.end(); ++i)
                registerWith(*i);
        }
    }

    inline
    Size Observer::unregisterWith(const boost::shared_ptr<Observable>& h) {
        if (h)
            h->unregisterObserver(this);
        return observables_.erase(h);
    }

    inline void Observer::unregisterWithAll() {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
        observables_.clear();
    }
}

#else

#include <boost/thread/locks.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/unordered_set.hpp>

namespace QuantLib {

    class Observable;

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer : public boost::enable_shared_from_this<Observer> {
              friend class Observable;
      public:
        // constructors, assignment, destructor
        Observer() {}
        Observer(const Observer&);
        Observer& operator=(const Observer&);
        virtual ~Observer();
        // observer interface
        std::pair<boost::unordered_set<boost::shared_ptr<Observable> >::iterator, bool>
        registerWith(const boost::shared_ptr<Observable>&);
        /*! register with all observables of a given observer. Note
            that this does not include registering with the observer
            itself. */
        void registerWithObservables(const boost::shared_ptr<Observer>&);
        Size unregisterWith(const boost::shared_ptr<Observable>&);
        void unregisterWithAll();
        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        virtual void update() = 0;
      private:

        class Proxy {
          public:
            Proxy(Observer* const observer)
             : active_  (true),
               observer_(observer) {
            }

            void update() const {
                boost::lock_guard<boost::recursive_mutex> lock(mutex_);
                if (active_) {
                    const boost::weak_ptr<Observer> o
                        = observer_->weak_from_this();
                    if (!o._empty()) {
                        const boost::shared_ptr<Observer> obs(o.lock());
                        if (obs)
                            obs->update();
                    }
                    else {
                        observer_->update();
                    }
                }
            }

            void deactivate() {
                boost::lock_guard<boost::recursive_mutex> lock(mutex_);
                active_ = false;
            }

        private:
            bool active_;
            mutable boost::recursive_mutex mutex_;
            Observer* const observer_;
        };

        boost::shared_ptr<Proxy> proxy_;
        mutable boost::recursive_mutex mutex_;

        boost::unordered_set<boost::shared_ptr<Observable> > observables_;
        typedef boost::unordered_set<boost::shared_ptr<Observable> >::iterator iterator;
    };

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
      public:
        // constructors, assignment, destructor
        Observable() {}
        Observable(const Observable&);
        Observable& operator=(const Observable&);
        virtual ~Observable() {}
        /*! This method should be called at the end of non-const methods
            or when the programmer desires to notify any changes.
        */
        void notifyObservers();
      private:
        void registerObserver(const boost::shared_ptr<Observer::Proxy>&);
        void unregisterObserver(const boost::shared_ptr<Observer::Proxy>&);

        typedef boost::signals2::signal<void()> signal_type;
        signal_type sig_;
       };


    // inline definitions

    inline Observable::Observable(const Observable&) {
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
    inline Observable& Observable::operator=(const Observable& o) {
        // as above, the observer set is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
    }

    inline void Observable::registerObserver(
        const boost::shared_ptr<Observer::Proxy>& observerProxy) {

        signal_type::slot_type slot(&Observer::Proxy::update,
                                    observerProxy.get());
        sig_.connect(slot.track(observerProxy));
    }

    inline void Observable::unregisterObserver(
        const boost::shared_ptr<Observer::Proxy>& observerProxy) {

        sig_.disconnect(boost::bind(&Observer::Proxy::update,
                        observerProxy.get()));
    }

    inline void Observable::notifyObservers() {
        sig_();
    }


    inline Observer::Observer(const Observer& o) {
        proxy_.reset(new Proxy(this));

        {
             boost::lock_guard<boost::recursive_mutex> lock(o.mutex_);
             observables_ = o.observables_;
        }

        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->registerObserver(proxy_);
    }

    inline Observer& Observer::operator=(const Observer& o) {
        boost::lock_guard<boost::recursive_mutex> lock(mutex_);
        if (!proxy_) {
            proxy_.reset(new Proxy(this));
        }

        iterator i;
        for (i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(proxy_);

        {
            boost::lock_guard<boost::recursive_mutex> lock(o.mutex_);
            observables_ = o.observables_;
        }
        for (i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->registerObserver(proxy_);

        return *this;
    }

    inline Observer::~Observer() {
        boost::lock_guard<boost::recursive_mutex> lock(mutex_);
        if (proxy_)
            proxy_->deactivate();

        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(proxy_);
    }

    inline std::pair<boost::unordered_set<boost::shared_ptr<Observable> >::iterator, bool>
    Observer::registerWith(const boost::shared_ptr<Observable>& h) {
        boost::lock_guard<boost::recursive_mutex> lock(mutex_);
        if (!proxy_) {
            proxy_.reset(new Proxy(this));
        }

        if (h) {
            h->registerObserver(proxy_);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
    }

    inline void
    Observer::registerWithObservables(const boost::shared_ptr<Observer>& o) {
        if (o) {
            boost::lock_guard<boost::recursive_mutex> lock(o->mutex_);

            for (iterator i = o->observables_.begin();
                 i != o->observables_.end(); ++i)
                registerWith(*i);
        }
    }

    inline
    Size Observer::unregisterWith(const boost::shared_ptr<Observable>& h) {
        boost::lock_guard<boost::recursive_mutex> lock(mutex_);

        if (h)  {
            QL_REQUIRE(proxy_, "unregister called without a proxy");
            h->unregisterObserver(proxy_);
        }

        return observables_.erase(h);
    }

    inline void Observer::unregisterWithAll() {
        boost::lock_guard<boost::recursive_mutex> lock(mutex_);

        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(proxy_);

        observables_.clear();
    }
}
#endif
#endif
