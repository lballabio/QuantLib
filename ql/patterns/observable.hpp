/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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
#include <boost/shared_ptr.hpp>
#include <list>

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
        void registerObserver(Observer*);
        void unregisterObserver(Observer*);
        std::list<Observer*> observers_;
        typedef std::list<Observer*>::iterator iterator;
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
        void registerWith(const boost::shared_ptr<Observable>&);
        void unregisterWith(const boost::shared_ptr<Observable>&);
        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        virtual void update() = 0;
      private:
        std::list<boost::shared_ptr<Observable> > observables_;
        typedef std::list<boost::shared_ptr<Observable> >::iterator iterator;
    };


    // inline definitions

    inline Observable::Observable(const Observable&) {
        // the observer list is not copied; no observer asked to
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
        // as above, the observer list is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
    }

    inline void Observable::registerObserver(Observer* o) {
        observers_.push_front(o);
    }

    inline void Observable::unregisterObserver(Observer* o) {
        iterator i = std::find(observers_.begin(),observers_.end(),o);
        if (i != observers_.end())
            observers_.erase(i);
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

    inline void Observer::registerWith(
                                     const boost::shared_ptr<Observable>& h) {
        if (h) {
            observables_.push_front(h);
            h->registerObserver(this);
        }
    }

    inline void Observer::unregisterWith(
                                    const boost::shared_ptr<Observable>& h) {
        if (h) {
            for (iterator i=observables_.begin();
                 i!=observables_.end();
                 ++i) {
                if (*i == h) {
                    (*i)->unregisterObserver(this);
                    observables_.erase(i);
                    return;
                }
            }
        }
    }

}


#endif
