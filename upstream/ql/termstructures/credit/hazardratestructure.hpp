/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file hazardratestructure.hpp
    \brief hazard-rate term structure
*/

#ifndef quantlib_hazard_rate_structure_hpp
#define quantlib_hazard_rate_structure_hpp

#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    //! Hazard-rate term structure
    /*! This abstract class acts as an adapter to
        DefaultProbabilityTermStructure allowing the programmer to implement
        only the <tt>hazardRateImpl(Time)</tt> method in derived classes.

        Survival/default probabilities and default densities are calculated
        from hazard rates.

        Hazard rates are defined with annual frequency and continuous
        compounding.

        \ingroup defaultprobabilitytermstructures
    */
    class HazardRateStructure : public DefaultProbabilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        HazardRateStructure(
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        HazardRateStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        HazardRateStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        //@}
      protected:
        /*! \name Calculations

            This method must be implemented in derived classes to
            perform the actual calculations. When it is called,
            range check has already been performed; therefore, it
            must assume that extrapolation is required.
        */
        //@{
        //! hazard rate calculation
        Real hazardRateImpl(Time) const override;
        //@}

        //! \name DefaultProbabilityTermStructure implementation
        //@{
        /*! survival probability calculation
            implemented in terms of the hazard rate \f$ h(t) \f$ as
            \f[
            S(t) = \exp\left( - \int_0^t h(\tau) d\tau \right).
            \f]

            \warning This default implementation uses numerical integration,
                     which might be inefficient and inaccurate.
                     Derived classes should override it if a more efficient
                     implementation is available.
        */
        Probability survivalProbabilityImpl(Time) const override;
        //! default density calculation
        Real defaultDensityImpl(Time) const override;
        //@}
    };

    // inline definitions

    inline Real HazardRateStructure::defaultDensityImpl(Time t) const {
        return hazardRateImpl(t)*survivalProbabilityImpl(t);
    }

}

#endif
