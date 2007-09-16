/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

/*! \file voltermstructure.hpp
    \brief Volatility term structure
*/

#ifndef quantlib_vol_term_structure_hpp
#define quantlib_vol_term_structure_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    //! Volatility term structure
    /*! This abstract class defines the interface of concrete
        volatility structures which will be derived from this one.

    */
    class VolatilityTermStructure : public TermStructure {
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
        VolatilityTermStructure(BusinessDayConvention bdc = Following,
                                const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        VolatilityTermStructure(const Date& referenceDate,
                                const Calendar& cal = Calendar(),
                                BusinessDayConvention bdc = Following,
                                const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        VolatilityTermStructure(Natural settlementDays,
                                const Calendar&,
                                BusinessDayConvention bdc = Following,
                                const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~VolatilityTermStructure() {}
        //! the business day convention used in tenor to date conversion
        BusinessDayConvention businessDayConvention() const;
        //! tenor to date conversion
        Date optionDateFromTenor(const Period&) const;
      private:
        BusinessDayConvention bdc_;
    };

    // inline definitions

    inline VolatilityTermStructure::VolatilityTermStructure(
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : TermStructure(dc) {}

    inline VolatilityTermStructure::VolatilityTermStructure(
                                                    const Date& referenceDate,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : TermStructure(referenceDate, cal, dc) {}

    inline VolatilityTermStructure::VolatilityTermStructure(
                                                    Natural settlementDays,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : TermStructure(settlementDays, cal, dc) {}

    inline BusinessDayConvention
    VolatilityTermStructure::businessDayConvention() const {
        return bdc_;
    }

    inline Date
    VolatilityTermStructure::optionDateFromTenor(const Period& p) const {
        return calendar().advance(referenceDate(),
                                  p,
                                  businessDayConvention());
    }

}

#endif
