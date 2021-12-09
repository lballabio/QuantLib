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

    //! Hazard-rate term structure
    /*! This abstract class acts as an adapter to
        DefaultProbabilityTermStructure allowing the programmer to implement
        only the <tt>survivalProbabilityImpl(Time)</tt> method in derived
        classes.

        Hazard rates and default densities are calculated
        from survival probabilities.

        \ingroup defaultprobabilitytermstructures
    */
    class SurvivalProbabilityStructure
        : public DefaultProbabilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        SurvivalProbabilityStructure(
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        SurvivalProbabilityStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        SurvivalProbabilityStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        //@}
      protected:
        //! \name DefaultProbabilityTermStructure implementation
        //@{
        //! instantaneous default density at a given time
        /*! implemented in terms of the survival probability \f$ S(t) \f$ as
            \f$ p(t) = -\frac{d}{dt} S(t). \f$

            \warning This implementation uses numerical differentiation,
                     which might be inefficient and inaccurate.
                     Derived classes should override it if a more efficient
                     implementation is available.
        */
        Real defaultDensityImpl(Time) const override;
        //@}
    };

}

#endif
