/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file event.hpp
    \brief Base class for events associated with a given date
*/

#ifndef quantlib_event_hpp
#define quantlib_event_hpp

#include <ql/date.hpp>
#include <ql/Patterns/observable.hpp>
#include <ql/Patterns/visitor.hpp>

namespace QuantLib {

    //! Base class for event
    /*! This class acts as a base class for the actual
        event implementations.
    */
    class Event : public Observable {
      public:
        virtual ~Event() {}
        //! \name Event interface
        //@{
        //! returns the date at which the event occurs
        virtual Date date() const = 0;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}

        //! returns true if an event has occurred on input date  
        // If QL_TODAYS_PAYMENT is true, then a payment event has not occurred
        // if the input date is the same as the event date, and so includeToday
        // should be defaulted to false.
        //
        // This should be the only place in the code that is affected directly
        // by QL_TODAYS_PAYMENT
        //
        // \today make QL_TODAYS_PAYMENT dynamically configurable

        bool hasOccurred(const Date &d,
#if QL_TODAYS_PAYMENTS
                         bool includeToday = false
#else
                         bool includeToday = true
#endif
                         ) const {
            if (includeToday) {
                return date() <= d;
            } else {
                return date() < d;
            }
        }
    };


    // inline definitions

    inline void Event::accept(AcyclicVisitor& v) {
        Visitor<Event>* v1 = dynamic_cast<Visitor<Event>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not an event visitor");
    }

    //! \name DateEvent implementation
    /*! This class is an adapter between a Date and the Event interface.
     * The main use of this class is to make the hasOccurred method of the Event
     * interface, visible to ordinary Dates.
     */

    class DateEvent : public Event, public Date {
    public:
        DateEvent() : Event(), Date() {};
        DateEvent(const Date& d) : Event(), Date(d) {};
        virtual ~DateEvent() {};
        virtual Date date() const {
            return *this;
        };
    };
}


#endif
