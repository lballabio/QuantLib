/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file survivalprobabilitystructure.hpp
    \brief survival-probability term structure
*/

#ifndef quantlib_survival_probability_structure_hpp
#define quantlib_survival_probability_structure_hpp

#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    //! survival-probability adapter for default-probability term structures
    class SurvivalProbabilityStructure : public DefaultProbabilityTermStructure {
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
        SurvivalProbabilityStructure(const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        SurvivalProbabilityStructure(const Date& referenceDate,
                                const Calendar& cal = Calendar(),
                                const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        SurvivalProbabilityStructure(Natural settlementDays,
                                const Calendar& cal,
                                const DayCounter& dc = DayCounter());
        //@}
      protected:
        //! instantaneous hazard rate at a given time
        /*! implemented in terms of the survival probability \f$ S(t) \f$ as
            \f$ h(t) = -[ln S(t)]/t. \f$
        */
        Real hazardRateImpl(Time) const;
        //! instantaneous default density at a given time
        /*! implemented in terms of the hazard rate \f$ h(t) \f$ and
            the survival probability \f$ S(t) \f$ as
            \f$ p(t) = h(t) S(t). \f$
        */
        Real defaultDensityImpl(Time) const;
    };

    // inline methods

    inline Real SurvivalProbabilityStructure::hazardRateImpl(Time t) const {
        if (t==0) {
            Time dt = 0.0001;
            return - std::log(survivalProbabilityImpl(dt))/dt;
        }
        return - std::log(survivalProbabilityImpl(t))/t;
    }

    inline
    Real SurvivalProbabilityStructure::defaultDensityImpl(Time t) const {
        return hazardRateImpl(t) * survivalProbabilityImpl(t);
    }

}

#endif
