/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2006 Ferdinando Ametrano

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

/*! \file swaptionvolstructure.hpp
    \brief Swaption volatility structure
*/

#ifndef quantlib_swaption_volatility_structure_hpp
#define quantlib_swaption_volatility_structure_hpp

#include <ql/termstructures/voltermstructure.hpp>

namespace QuantLib {

    class SmileSection;

    //! %Swaption-volatility structure
    /*! This abstract class defines the interface of concrete swaption
        volatility structures which will be derived from this one.
    */
    class SwaptionVolatilityStructure : public VolatilityTermStructure {
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
        SwaptionVolatilityStructure(const Calendar& calendar = Calendar(),
                                    const DayCounter& dc = DayCounter(),
                                    BusinessDayConvention bdc = Following);
        //! initialize with a fixed reference date
        SwaptionVolatilityStructure(const Date& referenceDate,
                                    const Calendar& calendar = Calendar(),
                                    const DayCounter& dc = DayCounter(),
                                    BusinessDayConvention bdc = Following);
        //! calculate the reference date based on the global evaluation date
        SwaptionVolatilityStructure(Natural settlementDays,
                                    const Calendar&,
                                    const DayCounter& dc = DayCounter(),
                                    BusinessDayConvention bdc = Following);
        //@}
        virtual ~SwaptionVolatilityStructure() {}
        //! \name Volatility, variance and smile
        //@{
        //! returns the volatility for a given option tenor and swap tenor
        Volatility volatility(const Period& optionTenor,
                              const Period& swapTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option date and swap tenor
        Volatility volatility(const Date& optionDate,
                              const Period& swapTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option time and swapLength
        Volatility volatility(Time optionTime,
                              Time swapLength,
                              Rate strike,
                              bool extrapolate = false) const;

        //! returns the Black variance for a given option tenor and swap tenor
        Real blackVariance(const Period& optionTenor,
                           const Period& swapTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given option date and swap tenor
        Real blackVariance(const Date& optionDate,
                           const Period& swapTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given option time and swapLength
        Real blackVariance(Time optionTime,
                           Time swapLength,
                           Rate strike,
                           bool extrapolate = false) const;

        //! returns the smile for a given option tenor and swap tenor
        boost::shared_ptr<SmileSection> smileSection(const Period& optionTenor,
                                                     const Period& swapTenor,
                                                     bool extr = false) const;
        //! returns the smile for a given option date and swap tenor
        boost::shared_ptr<SmileSection> smileSection(const Date& optionDate,
                                                     const Period& swapTenor,
                                                     bool extr = false) const;
        //! returns the smile for a given option time and swapLength
        boost::shared_ptr<SmileSection> smileSection(Time optionTime,
                                                     Time swapLength,
                                                     bool extr = false) const;
        //@}
        //! \name Limits
        //@{
        //! the largest length for which the term structure can return vols
        virtual const Period& maxSwapTenor() const = 0;
        //! the largest swapLength for which the term structure can return vols
        Time maxSwapLength() const;
        //@}
        //! implements the conversion between swap tenor and swap (time) length
        Time swapLength(const Period& swapTenor) const;
        //! implements the conversion between swap dates and swap (time) length
        Time swapLength(const Date& start,
                        const Date& end) const;
      protected:
        virtual boost::shared_ptr<SmileSection> smileSectionImpl(
                                                const Date& optionDate,
                                                const Period& swapTenor) const;
        virtual boost::shared_ptr<SmileSection> smileSectionImpl(
                                                Time optionTime,
                                                Time swapLength) const = 0;
        virtual Volatility volatilityImpl(const Date& optionDate,
                                          const Period& swapTenor,
                                          Rate strike) const;
        virtual Volatility volatilityImpl(Time optionTime,
                                          Time swapLength,
                                          Rate strike) const = 0;
        void checkSwapTenor(const Period& swapTenor,
                            bool extrapolate) const;
        void checkSwapTenor(Time swapLength,
                            bool extrapolate) const;
    };

    // inline definitions

    // 1. Period-based methods convert Period to Date and then
    //    use the equivalent Date-based methods
    inline Volatility
    SwaptionVolatilityStructure::volatility(const Period& optionTenor,
                                            const Period& swapTenor,
                                            Rate strike,
                                            bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return volatility(optionDate, swapTenor, strike, extrapolate);
    }

    inline
    Real SwaptionVolatilityStructure::blackVariance(const Period& optionTenor,
                                                    const Period& swapTenor,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return blackVariance(optionDate, swapTenor, strike, extrapolate);
    }

    inline boost::shared_ptr<SmileSection>
    SwaptionVolatilityStructure::smileSection(const Period& optionTenor,
                                              const Period& swapTenor,
                                              bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return smileSection(optionDate, swapTenor, extrapolate);
    }

    // 2. blackVariance methods rely on volatility methods
    inline
    Real SwaptionVolatilityStructure::blackVariance(const Date& optionDate,
                                                    const Period& swapTenor,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Volatility v = volatility(optionDate, swapTenor, strike, extrapolate);
        Time t = timeFromReference(optionDate);
        return v*v*t;
    }

    inline
    Real SwaptionVolatilityStructure::blackVariance(Time optionTime,
                                                    Time swapLength,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Volatility v = volatility(optionTime, swapLength, strike, extrapolate);
        return v*v*optionTime;
    }

    // 3. relying on xxxImpl methods
    inline Volatility
    SwaptionVolatilityStructure::volatility(const Date& optionDate,
                                            const Period& swapTenor,
                                            Rate strike,
                                            bool extrapolate) const {
        checkSwapTenor(swapTenor, extrapolate);
        checkRange(optionDate, extrapolate);
        checkStrike(strike, extrapolate);
        return volatilityImpl(optionDate, swapTenor, strike);
    }

    inline Volatility
    SwaptionVolatilityStructure::volatility(Time optionTime,
                                            Time swapLength,
                                            Rate strike,
                                            bool extrapolate) const {
        checkSwapTenor(swapLength, extrapolate);
        checkRange(optionTime, extrapolate);
        checkStrike(strike, extrapolate);
        return volatilityImpl(optionTime, swapLength, strike);
    }

    inline boost::shared_ptr<SmileSection>
    SwaptionVolatilityStructure::smileSection(const Date& optionDate,
                                              const Period& swapTenor,
                                              bool extrapolate) const {
        checkSwapTenor(swapTenor, extrapolate);
        checkRange(optionDate, extrapolate);
        return smileSectionImpl(optionDate, swapTenor);
    }

    inline boost::shared_ptr<SmileSection>
    SwaptionVolatilityStructure::smileSection(Time optionTime,
                                              Time swapLength,
                                              bool extrapolate) const {
        checkSwapTenor(swapLength, extrapolate);
        checkRange(optionTime, extrapolate);
        return smileSectionImpl(optionTime, swapLength);
    }

    // 4. default implementation of Date-based xxxImpl methods
    //    relying on the equivalent Time-based methods
    inline boost::shared_ptr<SmileSection>
    SwaptionVolatilityStructure::smileSectionImpl(const Date& optionDate,
                                                  const Period& swapT) const {
        return smileSectionImpl(timeFromReference(optionDate),
                                swapLength(swapT));
    }

    inline Volatility
    SwaptionVolatilityStructure::volatilityImpl(const Date& optionDate,
                                                const Period& swapTenor,
                                                Rate strike) const {
        return volatilityImpl(timeFromReference(optionDate),
                              swapLength(swapTenor),
                              strike);
    }

    inline Time SwaptionVolatilityStructure::maxSwapLength() const {
        return swapLength(maxSwapTenor());
    }

}

#endif
