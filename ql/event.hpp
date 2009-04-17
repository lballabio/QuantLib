/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2005 Joseph Wang

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

/*! \file event.hpp
    \brief Base class for events associated with a given date
*/

#ifndef quantlib_event_hpp
#define quantlib_event_hpp

#include <ql/time/date.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/patterns/visitor.hpp>

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

        //! returns true if an event has already occurred before a date
        /*! If QL_TODAYS_PAYMENTS is true, then a payment event has not
            occurred if the input date is the same as the event date,
            and so includeToday should be defaulted to true.

            This should be the only place in the code that is affected
            directly by QL_TODAYS_PAYMENTS
        */
        static bool hasOccurredFunction(const Date& d,
                                        const Date& refDate = Date(),
                                        #if defined(QL_TODAYS_PAYMENTS)
                                        bool includeToday = true);
                                        #else
                                        bool includeToday = false);
                                        #endif
        bool hasOccurred(const Date& refDate = Date(),
                         #if defined(QL_TODAYS_PAYMENTS)
                         bool includeToday = true) const {
                         #else
                         bool includeToday = false) const {
                         #endif
            return hasOccurredFunction(date(), refDate, includeToday);
        }
        //@}

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    // inline definitions

    inline void Event::accept(AcyclicVisitor& v) {
        Visitor<Event>* v1 = dynamic_cast<Visitor<Event>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not an event visitor");
    }

}


#endif
