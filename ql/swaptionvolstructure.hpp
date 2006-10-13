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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionvolstructure.hpp
    \brief Swaption volatility structure
*/

#ifndef quantlib_swaption_volatility_structure_hpp
#define quantlib_swaption_volatility_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Volatilities/smilesection.hpp>

namespace QuantLib {

    //! %Swaption-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        swaption volatility structures which will be derived from this one.
    */
    class SwaptionVolatilityStructure : public TermStructure {
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
        SwaptionVolatilityStructure();
        //! initialize with a fixed reference date
        SwaptionVolatilityStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        SwaptionVolatilityStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~SwaptionVolatilityStructure() {}
        //! \name Volatility and Variance
        //@{
        //! returns the volatility for a given option tenor and swap tenor
        Volatility volatility(const Period& optionTenor,
                              const Period& swapTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given exercise date and swap tenor
        Volatility volatility(const Date& exerciseDate,
                              const Period& swapTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given exercise date and swap tenor
        Real blackVariance(const Date& exerciseDate,
                           const Period& swapTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the volatility for a given starting time and length
        Volatility volatility(Time exerciseTime,
                              Time length,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given exercise time and length
        Real blackVariance(Time exerciseTime,
                           Time length,
                           Rate strike,
                           bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the latest start date for which the term structure can return vols
        virtual Date maxStartDate() const = 0;
        //! the largest length for which the term structure can return vols
        virtual Period maxLength() const = 0;
        //! the latest start time for which the term structure can return vols
        virtual Time maxStartTime() const;
        //! the largest length for which the term structure can return vols
        virtual Time maxTimeLength() const;
        //! the minimum strike for which the term structure can return vols
        virtual Rate minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Rate maxStrike() const = 0;
        //@}
        Date maxDate() const {
            return maxStartDate();
        }
        Time maxTime() const {
            return maxStartTime();
        }

        virtual boost::shared_ptr<SmileSection> smileSection(
                                                 const Date& start,
                                                 const Period& length) const {
            const std::pair<Time, Time> p = convertDates(start, length);
            return smileSection(p.first, p.second);
        }

        //! implements the conversion between dates and times
        virtual std::pair<Time,Time> convertDates(const Date& exerciseDate,
                                                  const Period& length) const;
      protected:
        //! return smile section
        virtual boost::shared_ptr<SmileSection> smileSection(
            Time start, Time length) const = 0;
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time exerciseTime,
                                          Time length,
                                          Rate strike) const = 0;
        virtual Volatility volatilityImpl(const Date& exerciseDate,
                                          const Period& length,
                                          Rate strike) const {
            const std::pair<Time, Time> p = convertDates(exerciseDate, length);
            return volatilityImpl(p.first, p.second, strike);
        }
        void checkRange(Time, Time, Rate strike, bool extrapolate) const;
        void checkRange(const Date& exerciseDate,
                        const Period& length,
                        Rate strike, bool extrapolate) const;
    };



    // inline definitions

    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure() {}

    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                   const Date& referenceDate)
    : TermStructure(referenceDate) {}

    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                             Integer settlementDays, const Calendar& calendar)
    : TermStructure(settlementDays,calendar) {}

    inline Volatility SwaptionVolatilityStructure::volatility(
                                                    const Period& optionTenor,
                                                    const Period& swapTenor,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Date exerciseDate = calendar().advance(referenceDate(),
                                               optionTenor,
                                               Following); //FIXME
        return volatility(exerciseDate, swapTenor, strike, extrapolate);
    }

    inline Volatility SwaptionVolatilityStructure::volatility(
                                                     const Date& exerciseDate,
                                                     const Period& swapTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(exerciseDate, swapTenor, strike, extrapolate);
        return volatilityImpl(exerciseDate, swapTenor, strike);
    }

    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     const Date& exerciseDate,
                                                     const Period& swapTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Volatility vol =
            volatility(exerciseDate, swapTenor, strike, extrapolate);
        const std::pair<Time, Time> p = convertDates(exerciseDate, swapTenor);
        return vol*vol*p.first;
    }

    inline Volatility SwaptionVolatilityStructure::volatility(
                                                     Time exerciseTime,
                                                     Time swapLength,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(exerciseTime, swapLength, strike, extrapolate);
        return volatilityImpl(exerciseTime, swapLength, strike);
    }

    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     Time exerciseTime,
                                                     Time swapLength,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(exerciseTime, swapLength, strike, extrapolate);
        Volatility vol = volatilityImpl(exerciseTime, swapLength, strike);
        return vol*vol*exerciseTime;
    }

    inline Time SwaptionVolatilityStructure::maxStartTime() const {
        return timeFromReference(maxStartDate());
    }

    inline Time SwaptionVolatilityStructure::maxTimeLength() const {
        return timeFromReference(referenceDate()+maxLength());
    }

    inline std::pair<Time,Time>
    SwaptionVolatilityStructure::convertDates(const Date& exerciseDate,
                                              const Period& length) const {
        Time startTime = timeFromReference(exerciseDate);
        Date end = exerciseDate + length;
        Time timeLength = dayCounter().yearFraction(exerciseDate,end);
        return std::make_pair(startTime,timeLength);
    }

    inline void SwaptionVolatilityStructure::checkRange(
             Time exerciseTime, Time length, Rate k, bool extrapolate) const {
        TermStructure::checkRange(exerciseTime, extrapolate);
        QL_REQUIRE(length >= 0.0,
                   "negative length (" << length << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   length <= maxTimeLength(),
                   "length (" << length << ") is past max curve length ("
                   << maxTimeLength() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

    inline void SwaptionVolatilityStructure::checkRange(
             const Date& exerciseDate, const Period& swapTenor,
             Rate k, bool extrapolate) const {
        TermStructure::checkRange(timeFromReference(exerciseDate),
                                  extrapolate);
        QL_REQUIRE(swapTenor.length() > 0,
                   "negative swap tenor (" << swapTenor << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   swapTenor <= maxLength(),
                   "swap tenor (" << swapTenor << ") is past max tenor ("
                   << maxLength() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}


#endif
