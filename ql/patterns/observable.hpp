/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2011, 2012 Ferdinando Ametrano

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

#include <set>

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
        typedef std::set<Observer*>::iterator iterator;
        std::pair<iterator, bool> registerObserver(Observer*);
        Size unregisterObserver(Observer*);
        std::set<Observer*> observers_;
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
        std::pair<std::set<boost::shared_ptr<Observable> >::iterator, bool>
                            registerWith(const boost::shared_ptr<Observable>&);
        Size unregisterWith(const boost::shared_ptr<Observable>&);
        void unregisterWithAll();
        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        virtual void update() = 0;
      private:
        std::set<boost::shared_ptr<Observable> > observables_;
        typedef std::set<boost::shared_ptr<Observable> >::iterator iterator;
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

    inline std::pair<std::set<Observer*>::iterator, bool>
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

    inline std::pair<std::set<boost::shared_ptr<Observable> >::iterator, bool>
    Observer::registerWith(const boost::shared_ptr<Observable>& h) {
        if (h) {
            h->registerObserver(this);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
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

#endif
