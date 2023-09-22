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

#include <ql/patterns/singleton.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/types.hpp>
#include <set>

#if !defined(QL_USE_STD_SHARED_PTR) && BOOST_VERSION < 107400

namespace std {

    template<typename T>
    struct hash<boost::shared_ptr<T>> {
        std::size_t operator()(const boost::shared_ptr<T>& ptr) const noexcept {
            return std::hash<typename boost::shared_ptr<T>::element_type*>()(ptr.get());
        }
    };

}

#endif

#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

namespace QuantLib {

    class Observer;
    class ObservableSettings;

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
        friend class ObservableSettings;
      public:
        // constructors, assignment, destructor
        Observable();
        Observable(const Observable&);
        Observable& operator=(const Observable&);
        // delete the move operations because the semantics are not yet clear
        Observable(Observable&&) = delete;
        Observable& operator=(Observable&&) = delete;
        virtual ~Observable() = default;
        /*! This method should be called at the end of non-const methods
            or when the programmer desires to notify any changes.
        */
        void notifyObservers();
      private:
        typedef std::set<Observer*> set_type;
        typedef set_type::iterator iterator;
        std::pair<iterator, bool> registerObserver(Observer*);
        Size unregisterObserver(Observer*);
        set_type observers_;
    };

    //! global repository for run-time library settings
    class ObservableSettings : public Singleton<ObservableSettings> {
        friend class Singleton<ObservableSettings>;
        friend class Observable;
      public:
        void disableUpdates(bool deferred=false);
        void enableUpdates();

        bool updatesEnabled() const;
        bool updatesDeferred() const;

      private:
        ObservableSettings() = default;

        typedef std::set<Observer*> set_type;
        typedef set_type::iterator iterator;

        void registerDeferredObservers(const Observable::set_type& observers);
        void unregisterDeferredObserver(Observer*);

        set_type deferredObservers_;

        bool updatesEnabled_ = true, updatesDeferred_ = false;
    };

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer { // NOLINT(cppcoreguidelines-special-member-functions)
      private:
        typedef std::set<ext::shared_ptr<Observable>> set_type;
      public:
        typedef set_type::iterator iterator;

        // constructors, assignment, destructor
        Observer() = default;
        Observer(const Observer&);
        Observer& operator=(const Observer&);
        virtual ~Observer();

        // observer interface
        std::pair<iterator, bool>
        registerWith(const ext::shared_ptr<Observable>&);

        /*! register with all observables of a given observer. Note
            that this does not include registering with the observer
            itself.
        */
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

}

#else

#include <boost/smart_ptr/owner_less.hpp>
#include <atomic>
#include <mutex>
#include <set>
#include <thread>

namespace QuantLib {

    class Observable;
    class ObservableSettings;

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer : public ext::enable_shared_from_this<Observer> {
        friend class Observable;
        friend class ObservableSettings;
      private:
        typedef std::set<ext::shared_ptr<Observable>> set_type;
      public:
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
            itself.
        */
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

            void update() const;

            void deactivate();

        private:
            bool active_;
            mutable std::recursive_mutex mutex_;
            Observer* const observer_;
        };

        ext::shared_ptr<Proxy> proxy_;
        mutable std::recursive_mutex mutex_;

        set_type observables_;
    };

    namespace detail {
        class Signal;
    }

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
        friend class ObservableSettings;
      private:
        typedef std::set<ext::shared_ptr<Observer::Proxy>> set_type;
      public:
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
        void unregisterObserver(
            const ext::shared_ptr<Observer::Proxy>& proxy, bool disconnect);

        ext::shared_ptr<detail::Signal> sig_;
        set_type observers_;
        mutable std::recursive_mutex mutex_;
    };

    //! global repository for run-time library settings
    class ObservableSettings : public Singleton<ObservableSettings> {
        friend class Singleton<ObservableSettings>;
        friend class Observable;

      public:
        void disableUpdates(bool deferred=false) {
            std::lock_guard<std::mutex> lock(mutex_);
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

        void registerDeferredObservers(const Observable::set_type& observers);
        void unregisterDeferredObserver(const ext::shared_ptr<Observer::Proxy>& proxy);

        set_type deferredObservers_;
        mutable std::mutex mutex_;

        enum UpdateType { UpdatesEnabled = 1, UpdatesDeferred = 2} ;
        std::atomic<int> updatesType_;
    };

}
#endif
#endif
