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

/*! \file defaulttermstructure.hpp
    \brief default-probability term structure
*/

#ifndef quantlib_default_term_structure_hpp
#define quantlib_default_term_structure_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    //! probability
    /*! \ingroup types */
    typedef Real Probability;


    //! default probability term structure
    class DefaultProbabilityTermStructure : public TermStructure {
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
        DefaultProbabilityTermStructure(const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        DefaultProbabilityTermStructure(const Date& referenceDate,
                                        const Calendar& cal = Calendar(),
                                        const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        DefaultProbabilityTermStructure(Natural settlementDays,
                                        const Calendar& cal,
                                        const DayCounter& dc = DayCounter());
        //@}
        //! \name Default probability
        //@{
        //! probability of default between the reference date and a given date
        Probability defaultProbability(const Date&,
                                       bool extrapolate = false) const;
        //! probability of default between t = 0 and a given time
        Probability defaultProbability(Time,
                                       bool extrapolate = false) const;
        //! probability of default between two given dates
        Probability defaultProbability(const Date&,
                                       const Date&,
                                       bool extrapolate = false) const;
        //! probability of default between two given times
        Probability defaultProbability(Time,
                                       Time,
                                       bool extrapolate = false) const;
        //@}
        //! \name Survival probability
        //@{
        //! probability of survival between the reference date and a given date
        Probability survivalProbability(const Date&,
                                        bool extrapolate = false) const;
        //! probability of survival between t = 0 and a given time
        Probability survivalProbability(Time,
                                        bool extrapolate = false) const;
        //@}
        //! \name Default density
        //@{
        //! default density at a given date
        Real defaultDensity(const Date&,
                            bool extrapolate = false) const;
        //! default density at a given time
        Real defaultDensity(Time,
                            bool extrapolate = false) const;
        //@}
        //! \name Hazard rate
        //@{
        //! hazard rate at a given date
        Real hazardRate(const Date&,
                        bool extrapolate = false) const;
        //! hazard rate at a given time
        Real hazardRate(Time,
                        bool extrapolate = false) const;
        //@}
      protected:
        /*! \name Calculations

            These methods must be implemented in derived classes to
            perform the actual calculations. When they are called,
            range check has already been performed; therefore, they
            must assume that extrapolation is required.
        */
        //@{
        //! probability of survival between reference time (t = 0) and a given time
        virtual Probability survivalProbabilityImpl(Time) const = 0;
        //! instantaneous default density at a given time
        virtual Real defaultDensityImpl(Time) const = 0;
        //! instantaneous hazard rate at a given time
        virtual Real hazardRateImpl(Time) const = 0;
        //@}
    };

}

#endif
