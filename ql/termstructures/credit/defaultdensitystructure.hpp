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
 <https://www.quantlib.org/license.shtml>.

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

    //! Default-density term structure
    /*! This abstract class acts as an adapter to
        DefaultProbabilityTermStructure allowing the programmer to implement
        only the <tt>defaultDensityImpl(Time)</tt> method in derived classes.

        Survival/default probabilities and hazard rates are calculated
        from default densities.

        \ingroup defaultprobabilitytermstructures
    */
    class DefaultDensityStructure : public DefaultProbabilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        DefaultDensityStructure(
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        DefaultDensityStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        DefaultDensityStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        //@}
      protected:
        //! \name DefaultProbabilityTermStructure implementation
        //@{
        //! survival probability calculation
        /*! implemented in terms of the default density \f$ p(t) \f$ as
            \f[
            S(t) = 1 - \int_0^t p(\tau) d\tau.
            \f]

            \warning This default implementation uses numerical integration,
                     which might be inefficient and inaccurate.
                     Derived classes should override it if a more efficient
                     implementation is available.
        */
        Probability survivalProbabilityImpl(Time) const override;
        //@}
    };

}

#endif
