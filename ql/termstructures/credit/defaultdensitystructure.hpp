/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file defaultdensitystructure.hpp
    \brief default-density term structure
*/

#ifndef quantlib_default_density_structure_hpp
#define quantlib_default_density_structure_hpp

#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    //! default-density adapter for default-probability term structures
    class DefaultDensityStructure : public DefaultProbabilityTermStructure {
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
        DefaultDensityStructure(const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        DefaultDensityStructure(const Date& referenceDate,
                                const Calendar& cal = Calendar(),
                                const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        DefaultDensityStructure(Natural settlementDays,
                                const Calendar& cal,
                                const DayCounter& dc = DayCounter());
        //@}
      protected:
        //! probability of survival between today (t = 0) and a given time
        /*! implemented in terms of the default density \f$ p(t) \f$ as
            \f[
            S(t) = 1 - \int_0^t p(\tau) d\tau.
            \f]

            \note This implementation uses numerical integration.
                  Derived classes should override it if a more
                  efficient formula is available.
        */
        Probability survivalProbabilityImpl(Time) const;
        //! instantaneous hazard rate at a given time
        /*! implemented in terms of the default density \f$ p(t) \f$ and
            the survival probability \f$ S(t) \f$ as
            \f$ h(t) = p(t)/S(t). \f$
        */
        Real hazardRateImpl(Time) const;
    };

}


#endif
