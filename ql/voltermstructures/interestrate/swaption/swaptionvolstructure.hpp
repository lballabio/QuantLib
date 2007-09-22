/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/voltermstructure.hpp>

namespace QuantLib {

    class SmileSection;

    //! %Swaption-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        swaption volatility structures which will be derived from this one.
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
        //! returns the volatility for a given option time and swapLength
        Volatility volatility(Time optionTime,
                              Time swapLength,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given option time and swapLength
        Real blackVariance(Time optionTime,
                           Time swapLength,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the smile for a given option time and swapLength
        boost::shared_ptr<SmileSection> smileSection(Time optionTime,
                                                     Time swapLength) const{
            return smileSectionImpl(optionTime,swapLength);
        };

        //! returns the volatility for a given option date and swap tenor
        Volatility volatility(const Date& optionDate,
                              const Period& swapTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given option date and swap tenor
        Real blackVariance(const Date& optionDate,
                           const Period& swapTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the smile for a given option date and swap tenor
        boost::shared_ptr<SmileSection> smileSection(
                                                 const Date& optionDate,
                                                 const Period& swapTenor) const;
        //! returns the volatility for a given option tenor and swap tenor
        Volatility volatility(const Period& optionTenor,
                              const Period& swapTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given option tenor and swap tenor
        Real blackVariance(const Period& optionTenor,
                           const Period& swapTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the smile for a given option tenor and swap tenor
        boost::shared_ptr<SmileSection> smileSection(
                                            const Period& optionTenor,
                                            const Period& swapTenor) const;
        //@}
        //! \name Limits
        //@{
        //! the largest length for which the term structure can return vols
        virtual const Period& maxSwapTenor() const = 0;
        //! the largest swapLength for which the term structure can return vols
        virtual Time maxSwapLength() const;
        //! the minimum strike for which the term structure can return vols
        virtual Rate minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Rate maxStrike() const = 0;
        //@}
        //! implements the conversion between dates and times
        virtual std::pair<Time,Time> convertDates(const Date& optionDate,
                                                  const Period& swapTenor) const;

      protected:
        //! return smile section
        virtual boost::shared_ptr<SmileSection> smileSectionImpl(
                                                Time optionTime,
                                                Time swapLength) const = 0;
        // overloaded (at least) in SwaptionVolCube2
        virtual boost::shared_ptr<SmileSection> smileSectionImpl(
                                                 const Date& optionDate,
                                                 const Period& swapTenor) const {
            const std::pair<Time, Time> p = convertDates(optionDate, swapTenor);
            return smileSectionImpl(p.first, p.second);
        }
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time optionTime,
                                          Time swapLength,
                                          Rate strike) const = 0;
        virtual Volatility volatilityImpl(const Date& optionDate,
                                          const Period& swapTenor,
                                          Rate strike) const {
            const std::pair<Time, Time> p = convertDates(optionDate, swapTenor);
            return volatilityImpl(p.first, p.second, strike);
        }
        void checkRange(Time, Time, Rate strike, bool extrapolate) const;
        void checkRange(const Date& optionDate,
                        const Period& swapTenor,
                        Rate strike, bool extrapolate) const;
      private:
        BusinessDayConvention bdc_;
    };



    // inline definitions

    // Volatility, variance (Time - Time - Rate)
    inline Volatility SwaptionVolatilityStructure::volatility(
                                                     Time optionTime,
                                                     Time swapLength,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(optionTime, swapLength, strike, extrapolate);
        return volatilityImpl(optionTime, swapLength, strike);
    }


    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     Time optionTime,
                                                     Time swapLength,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(optionTime, swapLength, strike, extrapolate);
        Volatility vol = volatilityImpl(optionTime, swapLength, strike);
        return vol*vol*optionTime;
    }

    // Volatility, variance and smile (Date - Tenor - Rate)
    inline Volatility SwaptionVolatilityStructure::volatility(
                                                     const Date& optionDate,
                                                     const Period& swapTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(optionDate, swapTenor, strike, extrapolate);
        return volatilityImpl(optionDate, swapTenor, strike);
    }

    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     const Date& optionDate,
                                                     const Period& swapTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Volatility vol =
            volatility(optionDate, swapTenor, strike, extrapolate);
        const std::pair<Time, Time> p = convertDates(optionDate, swapTenor);
        return vol*vol*p.first;
    }
    inline boost::shared_ptr<SmileSection>
        SwaptionVolatilityStructure::smileSection(
                                                 const Date& optionDate,
                                                 const Period& swapTenor) const {
           return smileSectionImpl(optionDate, swapTenor);
    }

    // Volatility, variance and smile (Tenor - Tenor - Rate)
    inline Volatility SwaptionVolatilityStructure::volatility(
                                                    const Period& optionTenor,
                                                    const Period& swapTenor,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return volatility(optionDate, swapTenor, strike, extrapolate);
    }

    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     const Period& optionTenor,
                                                     const Period& swapTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        Volatility vol =
            volatility(optionDate, swapTenor, strike, extrapolate);
        const std::pair<Time, Time> p = convertDates(optionDate, swapTenor);
        return vol*vol*p.first;
    }

    inline boost::shared_ptr<SmileSection>
    SwaptionVolatilityStructure::smileSection(const Period& optionTenor,
                                              const Period& swapTenor) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return smileSection(optionDate, swapTenor);
    }

    inline void SwaptionVolatilityStructure::checkRange(
             Time optionTime, Time swapLength, Rate k, bool extrapolate) const {
        TermStructure::checkRange(optionTime, extrapolate);
        QL_REQUIRE(swapLength >= 0.0,
                   "negative swapLength (" << swapLength << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   swapLength <= maxSwapLength(),
                   "swapLength (" << swapLength << ") is past max curve swapLength ("
                   << maxSwapLength() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}

#endif
