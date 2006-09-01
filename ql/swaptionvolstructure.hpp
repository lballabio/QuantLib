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

namespace QuantLib {

     //! swaption-volatility smile section
    /*! This class provides the volatility smile section
    */
    class VarianceSmileSection : std::unary_function<Rate, Real> {
      public:
        VarianceSmileSection(Time expiryTime,
              const std::vector<Rate>& strikes,
              const std::vector<Rate>& volatilities);

        VarianceSmileSection(Time timeToExpiry,
                             Rate forwardValue,
                             const std::vector<Rate>& strikes,
                             const std::vector<Volatility>& volatilities);

        VarianceSmileSection(
          const std::vector<Real>& sabrParameters,
          const std::vector<Rate>& strikes,
          const Time timeToExpiry);

        VarianceSmileSection(const VarianceSmileSection& o) {
                timeToExpiry_ = o.timeToExpiry_;
	            strikes_ = o.strikes_;
                volatilities_ = o.volatilities_;
                interpolation_ = o.interpolation_;
        };

        VarianceSmileSection& operator=(const VarianceSmileSection& o){
            timeToExpiry_ = o.timeToExpiry_;
	        strikes_ = o.strikes_;
            volatilities_ = o.volatilities_;
            interpolation_ = o.interpolation_;
	        return *this;
        };
        Real operator()(const Rate& strike) const;

        Real volatility(const Rate& strike) const{
            return std::sqrt(this->operator()(strike)/timeToExpiry_);
        };
      private:
        Time timeToExpiry_;
        std::vector<Rate> strikes_;
        std::vector<Volatility> volatilities_;
        boost::shared_ptr<Interpolation> interpolation_;
    };

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
        //! returns the volatility for a given starting date and length
        Volatility volatility(const Date& exerciseDate,
                              const Period& length,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given starting time and length
        Volatility volatility(Time exerciseTime,
                              Time length,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given starting date and length
        Real blackVariance(const Date& exerciseDate,
                           const Period& length,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given starting time and length
        Real blackVariance(Time exerciseTime,
                           Time length,
                           Rate strike,
                           bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the latest start date for which the term structure can return vols
        virtual Date maxStartDate() const = 0;
        //! the latest start time for which the term structure can return vols
        virtual Time maxStartTime() const;
        //! the largest length for which the term structure can return vols
        virtual Period maxLength() const = 0;
        //! the largest length for which the term structure can return vols
        virtual Time maxTimeLength() const;
        //! the minimum strike for which the term structure can return vols
        virtual Rate minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Rate maxStrike() const = 0;
        //@}
        Date maxDate() const { return maxStartDate(); }
        Time maxTime() const { return maxStartTime(); }
        virtual VarianceSmileSection smileSection(Date start, Period length) const;
        //! return smile section
        virtual VarianceSmileSection smileSection(Time start, Time length) const = 0;
        //! implements the conversion between dates and times
        virtual std::pair<Time,Time> convertDates(const Date& exerciseDate,
                                                  const Period& length) const;
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time exerciseTime, Time length,
                                          Rate strike) const = 0;
        void checkRange(Time, Time, Rate strike, bool extrapolate) const;
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
                                                     const Date& exerciseDate,
                                                     const Period& length,
                                                     Rate strike,
                                                     bool extrapolate) const {
        std::pair<Time,Time> times = convertDates(exerciseDate, length);
        checkRange(times.first, times.second, strike, extrapolate);
        return volatilityImpl(times.first, times.second, strike);
    }

    inline Volatility SwaptionVolatilityStructure::volatility(
                                                     Time exerciseTime,
                                                     Time length,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(exerciseTime, length, strike, extrapolate);
        return volatilityImpl(exerciseTime, length, strike);
    }

    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     const Date& exerciseDate,
                                                     const Period& length,
                                                     Rate strike,
                                                     bool extrapolate) const {
        std::pair<Time,Time> times = convertDates(exerciseDate, length);
        checkRange(times.first, times.second, strike, extrapolate);
        Volatility vol = volatilityImpl(times.first, times.second, strike);
        return vol*vol*times.first;
    }

    inline Real SwaptionVolatilityStructure::blackVariance(
                                                     Time exerciseTime,
                                                     Time length,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(exerciseTime, length, strike, extrapolate);
        Volatility vol = volatilityImpl(exerciseTime, length, strike);
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

    inline VarianceSmileSection SwaptionVolatilityStructure::smileSection(
                                            Date start, Period length) const {
        const std::pair<Time, Time> p = convertDates(start, length);
        return smileSection(p.first, p.second);
    }

}


#endif
