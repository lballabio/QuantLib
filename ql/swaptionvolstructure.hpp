
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionvolstructure.hpp
    \brief Swaption volatility structure

    \fullpath
    ql/%swaptionvolstructure.hpp
*/

// $Id$

#ifndef quantlib_swaption_volatility_structure_h
#define quantlib_swaption_volatility_structure_h

#include <ql/date.hpp>
#include <ql/daycounter.hpp>
#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    //! Swaption volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        swaption volatility structures which will be derived from this one.
    */
    class SwaptionVolatilityStructure : public Patterns::Observable {
      public:
        virtual ~SwaptionVolatilityStructure() {}
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the day counter used for internal date/time conversions
        virtual DayCounter dayCounter() const = 0;
        //! returns the volatility for a given starting date and length
        double volatility(const Date& start, const Period& length, 
                          Rate strike) const;
        //! returns the volatility for a given starting time and length
        double volatility(Time start, Time length, Rate strike) const;
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual double volatilityImpl(Time start, Time length, 
                                      Rate strike) const = 0;
    };


    // inline definitions
    
    double SwaptionVolatilityStructure::volatility(const Date& start, 
                                                   const Period& length, 
                                                   Rate strike) const {
        Time startTime = dayCounter().yearFraction(todaysDate(),start,
                                                   todaysDate(),start);
        Date end = start.plus(length);
        Time timeLength = dayCounter().yearFraction(start,end,start,end);
        return volatilityImpl(startTime,timeLength,strike);
    }

    double SwaptionVolatilityStructure::volatility(Time start, 
                                                   Time length, 
                                                   Rate strike) const {
        return volatilityImpl(start,length,strike);
    }
    
}


#endif
