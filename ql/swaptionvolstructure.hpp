
/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionvolstructure.hpp
    \brief Swaption volatility structure
*/

#ifndef quantlib_swaption_volatility_structure_hpp
#define quantlib_swaption_volatility_structure_hpp

#include <ql/basetermstructure.hpp>

namespace QuantLib {

    //! %Swaption-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        swaption volatility structures which will be derived from this one.
    */
    class SwaptionVolatilityStructure : public BaseTermStructure {
      public:
        /*! \name Constructors
            See the BaseTermStructure documentation for issues regarding
            constructors.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        SwaptionVolatilityStructure();
        #ifndef QL_DISABLE_DEPRECATED
        //! initialize with a fixed reference date
        SwaptionVolatilityStructure(const Date& today,
                                    const Date& referenceDate);
        #endif
        //! initialize with a fixed reference date
        SwaptionVolatilityStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        SwaptionVolatilityStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~SwaptionVolatilityStructure() {}
        //! \name Volatility
        //@{
        //! returns the volatility for a given starting date and length
        Volatility volatility(const Date& start, const Period& length,
                              Rate strike) const;
        //! returns the volatility for a given starting time and length
        Volatility volatility(Time start, Time length, Rate strike) const;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time start, Time length,
                                          Rate strike) const = 0;
        //! implements the conversion between dates and times
        virtual std::pair<Time,Time> convertDates(const Date& start,
                                                  const Period& length) const;
    };


    // inline definitions

    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure() {}

    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                   const Date& referenceDate)
    : BaseTermStructure(referenceDate) {}

    #ifndef QL_DISABLE_DEPRECATED
    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                 const Date& today, const Date& referenceDate)
    : BaseTermStructure(today,referenceDate) {}
    #endif

    inline SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                             Integer settlementDays, const Calendar& calendar)
    : BaseTermStructure(settlementDays,calendar) {}

    inline Volatility SwaptionVolatilityStructure::volatility(
                                                        const Date& start,
                                                        const Period& length,
                                                        Rate strike) const {
        std::pair<Time,Time> times = convertDates(start,length);
        return volatilityImpl(times.first,times.second,strike);
    }

    inline Volatility SwaptionVolatilityStructure::volatility(Time start,
                                                              Time length,
                                                              Rate strike)
                                                                     const {
        return volatilityImpl(start,length,strike);
    }

    inline std::pair<Time,Time>
    SwaptionVolatilityStructure::convertDates(const Date& start,
                                              const Period& length) const {
        Time startTime = timeFromReference(start);
        Date end = start + length;
        Time timeLength = dayCounter().yearFraction(start,end);
        return std::make_pair(startTime,timeLength);
    }

}


#endif
