/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
#include <ql/quote.hpp>

namespace QuantLib {

    //! Default probability term structure
    /*! This abstract class defines the interface of concrete
        credit structures which will be derived from this one.

        \ingroup defaultprobabilitytermstructures
    */
    class DefaultProbabilityTermStructure : public TermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        DefaultProbabilityTermStructure(
            const DayCounter& dc = DayCounter(),
            std::vector<Handle<Quote> > jumps = {},
            const std::vector<Date>& jumpDates = {});
        DefaultProbabilityTermStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dc = DayCounter(),
            std::vector<Handle<Quote> > jumps = {},
            const std::vector<Date>& jumpDates = {});
        DefaultProbabilityTermStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dc = DayCounter(),
            std::vector<Handle<Quote> > jumps = {},
            const std::vector<Date>& jumpDates = {});
        //@}

        /*! \name Survival probabilities

            These methods return the survival probability from the reference
            date until a given date or time.  In the latter case, the time
            is calculated as a fraction of year from the reference date.
        */
        //@{
        Probability survivalProbability(const Date& d,
                                        bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        Probability survivalProbability(Time t,
                                        bool extrapolate = false) const;
        //@}

        /*! \name Default probabilities

            These methods return the default probability from the reference
            date until a given date or time.  In the latter case, the time
            is calculated as a fraction of year from the reference date.
        */
        //@{
        Probability defaultProbability(const Date& d,
                                       bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        Probability defaultProbability(Time t,
                                       bool extrapolate = false) const;
        //! probability of default between two given dates
        Probability defaultProbability(const Date&,
                                       const Date&,
                                       bool extrapolate = false) const;
        //! probability of default between two given times
        Probability defaultProbability(Time,
                                       Time,
                                       bool extrapo = false) const;
        //@}

        /*! \name Default densities

            These methods return the default density at a given date or time.
            In the latter case, the time is calculated as a fraction of year
            from the reference date.
        */
        //@{
        Real defaultDensity(const Date& d,
                            bool extrapolate = false) const;
        Real defaultDensity(Time t,
                            bool extrapolate = false) const;
        //@}

        /*! \name Hazard rates

            These methods returns the hazard rate at a given date or time.
            In the latter case, the time is calculated as a fraction of year
            from the reference date.
            
            Hazard rates are defined with annual frequency and continuous
            compounding.
        */

        //@{
        Rate hazardRate(const Date& d,
                        bool extrapolate = false) const;
        Rate hazardRate(Time t,
                        bool extrapolate = false) const;
        //@}

        //! \name Jump inspectors
        //@{
        const std::vector<Date>& jumpDates() const;
        const std::vector<Time>& jumpTimes() const;
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}
      protected:
        /*! \name Calculations
            The first two methods must be implemented in derived classes to
            perform the actual calculations. When they are called,
            range check has already been performed; therefore, they
            must assume that extrapolation is required.
            The third method has a default implementation which can be
            overriden with a more efficient implementation in derived
            classes.
        */
        //@{
        //! survival probability calculation
        virtual Probability survivalProbabilityImpl(Time) const = 0;
        //! default density calculation
        virtual Real defaultDensityImpl(Time) const = 0;
        //! hazard rate calculation
        virtual Real hazardRateImpl(Time) const;
        //@}
      private:
        // methods
        void setJumps();
        // data members
        std::vector<Handle<Quote> > jumps_;
        std::vector<Date> jumpDates_;
        std::vector<Time> jumpTimes_;
        Size nJumps_;
        Date latestReference_;
    };

    // inline definitions

    inline
    Probability DefaultProbabilityTermStructure::survivalProbability(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return survivalProbability(timeFromReference(d), extrapolate);
    }

    inline
    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return 1.0 - survivalProbability(d, extrapolate);
    }

    inline
    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     Time t,
                                                     bool extrapolate) const {
        return 1.0 - survivalProbability(t, extrapolate);
    }

    inline
    Real DefaultProbabilityTermStructure::defaultDensity(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return defaultDensity(timeFromReference(d), extrapolate);
    }

    inline
    Real DefaultProbabilityTermStructure::defaultDensity(
                                                     Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);
        return defaultDensityImpl(t);
    }

    inline
    Rate DefaultProbabilityTermStructure::hazardRate(const Date& d,
                                                     bool extrapolate) const {
        return hazardRate(timeFromReference(d), extrapolate);
    }

   inline
    Rate DefaultProbabilityTermStructure::hazardRateImpl(Time t) const {
        Probability S = survivalProbability(t, true);
        return S == 0.0 ? Rate(0.0) : defaultDensity(t, true)/S;
    }

    inline Rate DefaultProbabilityTermStructure::hazardRate(Time t,
                                                            bool extrapolate) const {
        checkRange(t, extrapolate);
        return hazardRateImpl(t);
    }

    inline
    const std::vector<Date>&
    DefaultProbabilityTermStructure::jumpDates() const {
        return this->jumpDates_;
    }

    inline
    const std::vector<Time>&
    DefaultProbabilityTermStructure::jumpTimes() const {
        return this->jumpTimes_;
    }

    inline void DefaultProbabilityTermStructure::update() {
        TermStructure::update();
        if (referenceDate() != latestReference_)
            setJumps();
    }

}

#endif
