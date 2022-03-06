/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#ifndef quantlib_correl_term_structure_hpp
#define quantlib_correl_term_structure_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    // pretty much like the volatility TS, here the correlation range is 
    // obviously known in advance and theres no reference to a strike.

    /*! Abstract interface, derived correlations TS might have elements with 
    arbitrary dimensions.\par
    In principle there might be several extrapolation dimensions, at this 
    level we do not know how many or the nature of those dimensions (time, 
    strike...) 
    Equally we ignore at this level if the correlation is a number, 
    matrix. Rather than including an arbitrary size matrix this data
    structure is deferred in the hierarchy to enable potential optimizations
    on the data nature.
    */
    class CorrelationTermStructure : public TermStructure {
    public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        CorrelationTermStructure(const Calendar& cal,
                                 BusinessDayConvention bdc,
                                 const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        CorrelationTermStructure(const Date& referenceDate,
                                 const Calendar& cal,
                                 BusinessDayConvention bdc,
                                 const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        CorrelationTermStructure(Natural settlementDays,
                                 const Calendar& cal,
                                 BusinessDayConvention bdc,
                                 const DayCounter& dc = DayCounter());
        //@}
        BusinessDayConvention businessDayConvention() const;
        //! period/date conversion
        Date dateFromTenor(const Period&) const;
        //! The size of the squared correlation.
        virtual Size correlationSize() const = 0;
    private:
        BusinessDayConvention bdc_;
    };

    // inline definitions
    inline BusinessDayConvention
    CorrelationTermStructure::businessDayConvention() const {
        return bdc_;
    }

    inline Date
    CorrelationTermStructure::dateFromTenor(const Period& p) const {
        // swaption style, still holds here.
        return calendar().advance(referenceDate(),
                                  p,
                                  businessDayConvention());
    }
}

#endif


#ifndef id_6457d49a7e7cde2084789d1a11ba045f
#define id_6457d49a7e7cde2084789d1a11ba045f
inline bool test_6457d49a7e7cde2084789d1a11ba045f(const int* i) {
    return i != nullptr;
}
#endif
