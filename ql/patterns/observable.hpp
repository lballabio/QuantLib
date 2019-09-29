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
#include <ql/patterns/singleton.hpp>

#include <ql/shared_ptr.hpp>
#include <boost/unordered_set.hpp>


#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

namespace QuantLib {

    class Observer;
    class Observable;

    //! global repository for run-time library settings
    class ObservableSettings : public Singleton<ObservableSettings> {
        friend class Singleton<ObservableSettings>;
        friend class Observable;
      public:
        void disableUpdates(bool deferred=false) {
            updatesEnabled_  = false;
            updatesDeferred_ = deferred;
        }
        void enableUpdates();

        bool updatesEnabled()  {return updatesEnabled_;}
        bool updatesDeferred() {return updatesDeferred_;}
      private:
        ObservableSettings()
        : updatesEnabled_(true),
          updatesDeferred_(false) {}

        void registerDeferredObservers(
            const boost::unordered_set<Observer*>& observers);
        void unregisterDeferredObserver(Observer*);

        typedef boost::unordered_set<Observer*> set_type;
        typedef set_type::iterator iterator;
        set_type deferredObservers_;

        bool updatesEnabled_,  updatesDeferred_;
    };

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
      public:
        // constructors, assignment, destructor
        Observable() : settings_(ObservableSettings::instance()) {}
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
        ObservableSettings& settings_;
    };

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer {
      public:
        typedef boost::unordered_set<ext::shared_ptr<Observable> > set_type;
        typedef set_type::iterator iterator;

        // constructors, assignment, destructor
        Observer() {}
        Observer(const Observer&);
        Observer& operator=(const Observer&);
        virtual ~Observer();

        // observer interface
        std::pair<iterator, bool>
            registerWith(const ext::shared_ptr<Observable>&);

        /*! register with all observables of a given observer. Note
            that this does not include registering with the observer
            itself. */
        void registerWithObservables(const ext::shared_ptr<Observer>&);
        Size unregisterWith(const ext::shared_ptr<Observable>&);
        void unregisterWithAll();

        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        virtual void update() = 0;

        /*! This method allows to explicitly update the instance itself
          and nested observers. If notifications are disabled a call to
          this method ensures an update of such nested observers. It
          should be implemented in derived classes whenever applicable */
        virtual void deepUpdate();

      private:
        set_type observables_;
    };


    // inline definitions

    inline void ObservableSettings::registerDeferredObservers(
        const boost::unordered_set<Observer*>& observers) {
        if (updatesDeferred()) {
            deferredObservers_.insert(observers.begin(), observers.end());
        }
    }

    inline void ObservableSettings::unregisterDeferredObserver(Observer* o) {
        deferredObservers_.erase(o);
    }

    inline Observable::Observable(const Observable&)
    : settings_(ObservableSettings::instance()) {
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
        if (settings_.updatesDeferred())
            settings_.unregisterDeferredObserver(o);

        return observers_.erase(o);
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

    inline std::pair<Observer::iterator, bool>
    Observer::registerWith(const ext::shared_ptr<Observable>& h) {
        if (h) {
            h->registerObserver(this);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
    }

    inline void
    Observer::registerWithObservables(const ext::shared_ptr<Observer> &o) {
        if (o) {
            iterator i;
            for (i = o->observables_.begin(); i != o->observables_.end(); ++i)
                registerWith(*i);
        }
    }

    inline
    Size Observer::unregisterWith(const ext::shared_ptr<Observable>& h) {
        if (h)
            h->unregisterObserver(this);
        return observables_.erase(h);
    }

    inline void Observer::unregisterWithAll() {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
        observables_.clear();
    }

    inline void Observer::deepUpdate() {
        update();
    }

}

#else

#include <boost/atomic.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/smart_ptr/owner_less.hpp>
#include <set>



namespace QuantLib {
    class Observable;
    class ObservableSettings;

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer : public ext::enable_shared_from_this<Observer> {
        friend class Observable;
        friend class ObservableSettings;
      public:
        typedef boost::unordered_set<ext::shared_ptr<Observable> > set_type;
        typedef set_type::iterator iterator;

        // constructors, assignment, destructor
        Observer() {}
        Observer(const Observer&);
        Observer& operator=(const Observer&);
        virtual ~Observer();
        // observer interface
        std::pair<iterator, bool>
            registerWith(const ext::shared_ptr<Observable>&);
        /*! register with all observables of a given observer. Note
            that this does not include registering with the observer
            itself. */
        void registerWithObservables(const ext::shared_ptr<Observer>&);
        Size unregisterWith(const ext::shared_ptr<Observable>&);
        void unregisterWithAll();

        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        virtual void update() = 0;

        /*! This method allows to explicitly update the instance itself
          and nested observers. If notifications are disabled a call to
          this method ensures an update of such nested observers. It
          should be implemented in derived classes whenever applicable */
        virtual void deepUpdate();

      private:

        class Proxy {
          public:
            explicit Proxy(Observer* const observer)
             : active_  (true),
               observer_(observer) {
            }

            void update() const {
                boost::lock_guard<boost::recursive_mutex> lock(mutex_);
                if (active_) {
                    // c++17 is required if used with std::shared_ptr<T>
                    const ext::weak_ptr<Observer> o
                        = observer_->weak_from_this();

                    //check for empty weak reference
                    //https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
                    const ext::weak_ptr<Observer> empty;
                    if (o.owner_before(empty) || empty.owner_before(o)) {
                        const ext::shared_ptr<Observer> obs(o.lock());
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

        ext::shared_ptr<Proxy> proxy_;
        mutable boost::recursive_mutex mutex_;

        set_type observables_;
    };

	namespace detail {
		class Signal;
	}

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
      public:
        typedef boost::unordered_set<ext::shared_ptr<Observer::Proxy> >
            set_type;
        typedef set_type::iterator iterator;

        // constructors, assignment, destructor
        Observable();
        Observable(const Observable&);
        Observable& operator=(const Observable&);
        virtual ~Observable() {}
        /*! This method should be called at the end of non-const methods
            or when the programmer desires to notify any changes.
        */
        void notifyObservers();
      private:
        void registerObserver(const ext::shared_ptr<Observer::Proxy>&);
        void unregisterObserver(const ext::shared_ptr<Observer::Proxy>&);

        ext::shared_ptr<detail::Signal> sig_;

        set_type observers_;
        mutable boost::recursive_mutex mutex_;

        ObservableSettings& settings_;
    };

    //! global repository for run-time library settings
    class ObservableSettings : public Singleton<ObservableSettings> {
        friend class Singleton<ObservableSettings>;
        friend class Observable;

    public:
        void disableUpdates(bool deferred=false) {
            boost::lock_guard<boost::mutex> lock(mutex_);
            updatesType_ = (deferred) ? UpdatesDeferred : 0;
        }
        void enableUpdates();

        bool updatesEnabled()  {return (updatesType_ & UpdatesEnabled) != 0; }
        bool updatesDeferred() {return (updatesType_ & UpdatesDeferred) != 0; }
      private:
        ObservableSettings() : updatesType_(UpdatesEnabled) {}

        typedef std::set<ext::weak_ptr<Observer::Proxy>,
                         boost::owner_less<ext::weak_ptr<Observer::Proxy> > >
            set_type;
        typedef set_type::iterator iterator;

        void registerDeferredObservers(const Observable::set_type& observers);
        void unregisterDeferredObserver(
            const ext::shared_ptr<Observer::Proxy>& proxy);

        set_type deferredObservers_;
        mutable boost::mutex mutex_;

        enum UpdateType { UpdatesEnabled = 1, UpdatesDeferred = 2} ;
        boost::atomic<int> updatesType_;
    };


    // inline definitions

    inline void ObservableSettings::registerDeferredObservers(
        const Observable::set_type& observers) {
        deferredObservers_.insert(observers.begin(), observers.end());
    }

    inline void ObservableSettings::unregisterDeferredObserver(
        const ext::shared_ptr<Observer::Proxy>& o) {
        deferredObservers_.erase(o);
    }

    inline void ObservableSettings::enableUpdates() {
        boost::lock_guard<boost::mutex> lock(mutex_);

        // if there are outstanding deferred updates, do the notification
        updatesType_ = UpdatesEnabled;

        if (deferredObservers_.size()) {
            bool successful = true;
            std::string errMsg;

            for (iterator i=deferredObservers_.begin();
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
    inline Observable& Observable::operator=(const Observable& o) {
        // as above, the observer set is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
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

    inline std::pair<Observer::iterator, bool>
    Observer::registerWith(const ext::shared_ptr<Observable>& h) {
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
    Observer::registerWithObservables(const ext::shared_ptr<Observer>& o) {
        if (o) {
            boost::lock_guard<boost::recursive_mutex> lock(o->mutex_);

            for (iterator i = o->observables_.begin();
                 i != o->observables_.end(); ++i)
                registerWith(*i);
        }
    }

    inline
    Size Observer::unregisterWith(const ext::shared_ptr<Observable>& h) {
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

    inline void Observer::deepUpdate() {
        update();
    }
}
#endif
#endif
