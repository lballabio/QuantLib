
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file observable.hpp
    \brief observer/observable pattern
*/

#ifndef quantlib_observable_h
#define quantlib_observable_h

#include <ql/handle.hpp>
#include <list>

namespace QuantLib {

    //! Object that notifies its changes to a set of observables
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
      public:
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
        template <class T>
        void registerWith(const boost::shared_ptr<T>& h) {
            if (h) {
                boost::shared_ptr<Observable> obs = h;
                observables_.push_front(obs);
                obs->registerObserver(this);
            }
        }
        template <class T>
        void unregisterWith(const boost::shared_ptr<T>& h) {
            if (h) {
                boost::shared_ptr<Observable> obs = h;
                for (iterator i=observables_.begin(); 
                              i!=observables_.end();
                              ++i) {
                    if (*i == obs) {
                        (*i)->unregisterObserver(this);
                        observables_.erase(i);
                        return;
                    }
                }
            }
        }
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

    inline void Observable::registerObserver(Observer* o) {
        observers_.push_front(o);
    }

    inline void Observable::unregisterObserver(Observer* o) {
        iterator i = std::find(observers_.begin(),observers_.end(),o);
        if (i != observers_.end())
            observers_.erase(i);
    }

    inline void Observable::notifyObservers() {
        for (iterator i=observers_.begin(); i!=observers_.end(); ++i)
            (*i)->update();
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

}


#endif
