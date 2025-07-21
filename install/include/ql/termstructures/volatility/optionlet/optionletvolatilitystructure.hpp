/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2015 Michael von den Driesch

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

/*! \file optionletvolatilitystructure.hpp
    \brief optionlet (caplet/floorlet) volatility structure
*/

#ifndef quantlib_optionlet_volatility_structure_hpp
#define quantlib_optionlet_volatility_structure_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    class SmileSection;

    //! Optionlet (caplet/floorlet) volatility structure
    /*! This class is purely abstract and defines the interface of
        concrete structures which will be derived from this one.
    */
    class OptionletVolatilityStructure : public VolatilityTermStructure {
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
        OptionletVolatilityStructure(BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        OptionletVolatilityStructure(const Date& referenceDate,
                                     const Calendar& cal,
                                     BusinessDayConvention bdc,
                                     const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        OptionletVolatilityStructure(Natural settlementDays,
                                     const Calendar&,
                                     BusinessDayConvention bdc,
                                     const DayCounter& dc = DayCounter());
        //@}
        ~OptionletVolatilityStructure() override = default;
        //! \name Volatility and Variance
        //@{
        //! returns the volatility for a given option tenor and strike rate
        Volatility volatility(const Period& optionTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option date and strike rate
        Volatility volatility(const Date& optionDate,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option time and strike rate
        Volatility volatility(Time optionTime,
                              Rate strike,
                              bool extrapolate = false) const;

        //! returns the Black variance for a given option tenor and strike rate
        Real blackVariance(const Period& optionTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given option date and strike rate
        Real blackVariance(const Date& optionDate,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given option time and strike rate
        Real blackVariance(Time optionTime,
                           Rate strike,
                           bool extrapolate = false) const;

        //! returns the smile for a given option tenor
        ext::shared_ptr<SmileSection> smileSection(const Period& optionTenor,
                                                     bool extr = false) const;
        //! returns the smile for a given option date
        ext::shared_ptr<SmileSection> smileSection(const Date& optionDate,
                                                     bool extr = false) const;
        //! returns the smile for a given option time
        ext::shared_ptr<SmileSection> smileSection(Time optionTime,
                                                     bool extr = false) const;
        //@}
        virtual VolatilityType volatilityType() const;
        virtual Real displacement() const;

      protected:
        virtual ext::shared_ptr<SmileSection> smileSectionImpl(
                                                const Date& optionDate) const;
        //! implements the actual smile calculation in derived classes
        virtual ext::shared_ptr<SmileSection> smileSectionImpl(
                                                    Time optionTime) const = 0;
        virtual Volatility volatilityImpl(const Date& optionDate,
                                          Rate strike) const;
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time optionTime,
                                          Rate strike) const = 0;
    };

    // inline definitions

    // 1. Period-based methods convert Period to Date and then
    //    use the equivalent Date-based methods
    inline Volatility
    OptionletVolatilityStructure::volatility(const Period& optionTenor,
                                             Rate strike,
                                             bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return volatility(optionDate, strike, extrapolate);
    }

    inline
    Real OptionletVolatilityStructure::blackVariance(const Period& optionTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return blackVariance(optionDate, strike, extrapolate);
    }

    inline ext::shared_ptr<SmileSection>
    OptionletVolatilityStructure::smileSection(const Period& optionTenor,
                                               bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return smileSection(optionDate, extrapolate);
    }

    // 2. blackVariance methods rely on volatility methods
    inline
    Real OptionletVolatilityStructure::blackVariance(const Date& optionDate,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Volatility v = volatility(optionDate, strike, extrapolate);
        Time t = timeFromReference(optionDate);
        return v*v*t;
    }

    inline
    Real OptionletVolatilityStructure::blackVariance(Time optionTime,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Volatility v = volatility(optionTime, strike, extrapolate);
        return v*v*optionTime;
    }

    // 3. relying on xxxImpl methods
    inline Volatility
    OptionletVolatilityStructure::volatility(const Date& optionDate,
                                             Rate strike,
                                             bool extrapolate) const {
        checkRange(optionDate, extrapolate);
        checkStrike(strike, extrapolate);
        return volatilityImpl(optionDate, strike);
    }

    inline Volatility
    OptionletVolatilityStructure::volatility(Time optionTime,
                                             Rate strike,
                                             bool extrapolate) const {
        checkRange(optionTime, extrapolate);
        checkStrike(strike, extrapolate);
        return volatilityImpl(optionTime, strike);
    }

    inline ext::shared_ptr<SmileSection>
    OptionletVolatilityStructure::smileSection(const Date& optionDate,
                                               bool extrapolate) const {
        checkRange(optionDate, extrapolate);
        return smileSectionImpl(optionDate);
    }

    inline ext::shared_ptr<SmileSection>
    OptionletVolatilityStructure::smileSection(Time optionTime,
                                               bool extrapolate) const {
        checkRange(optionTime, extrapolate);
        return smileSectionImpl(optionTime);
    }

    // 4. default implementation of Date-based xxxImpl methods
    //    relying on the equivalent Time-based methods
    inline ext::shared_ptr<SmileSection>
    OptionletVolatilityStructure::smileSectionImpl(const Date& optionDate) const {
        return smileSectionImpl(timeFromReference(optionDate));
    }

    inline Volatility
    OptionletVolatilityStructure::volatilityImpl(const Date& optionDate,
                                                 Rate strike) const {
        return volatilityImpl(timeFromReference(optionDate), strike);
    }

    inline VolatilityType
    OptionletVolatilityStructure::volatilityType() const {
        return ShiftedLognormal;
    }

    inline Real OptionletVolatilityStructure::displacement() const {
        return 0.0;
    }
}

#endif
