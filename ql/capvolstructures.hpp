
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file capvolstructures.hpp
    \brief Cap/Floor volatility structures
*/

#ifndef quantlib_cap_volatility_structures_h
#define quantlib_cap_volatility_structures_h

#include <ql/daycounter.hpp>
#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    //! Cap/floor flat volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapFlatVolatilityStructure : public Observable {
      public:
        virtual ~CapFlatVolatilityStructure() {}
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the settlement date
        virtual Date settlementDate() const = 0;
        //! returns the day counter used for internal date/time conversions
        virtual DayCounter dayCounter() const = 0;
        //! returns the volatility for a given end date and strike rate
        Volatility volatility(const Date& end, Rate strike) const;
        //! returns the volatility for a given cap/floor length and strike rate
        Volatility volatility(const Period& length, Rate strike) const;
        //! returns the volatility for a given end time and strike rate
        Volatility volatility(Time t, Rate strike) const;
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
    };

    //! Caplet/floorlet forward volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapletForwardVolatilityStructure : public Observable {
      public:
        virtual ~CapletForwardVolatilityStructure() {}
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the settlement date
        virtual Date settlementDate() const = 0;
        //! returns the day counter used for internal date/time conversions
        virtual DayCounter dayCounter() const = 0;
        //! returns the volatility for a given start date and strike rate
        Volatility volatility(const Date& start, Rate strike) const ;
        //! returns the volatility for a given start time and strike rate
        Volatility volatility(Time t, Rate strike) const;
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
    };

    // inline definitions

    inline Volatility CapFlatVolatilityStructure::volatility(const Date& end, 
                                                             Rate strike) 
                                                                     const {
        Date start = settlementDate();
        Time t = dayCounter().yearFraction(start,end,start,end);
        return volatilityImpl(t,strike);
    }

    inline Volatility CapFlatVolatilityStructure::volatility(
                                                        const Period& length, 
                                                        Rate strike) const {
        Date start = settlementDate();
        Date end = start.plus(length);
        Time t = dayCounter().yearFraction(start,end,start,end);
        return volatilityImpl(t,strike);
    }

    inline Volatility CapFlatVolatilityStructure::volatility(Time t, 
                                                             Rate strike)
                                                                     const {
        return volatilityImpl(t,strike);
    }


    inline Volatility CapletForwardVolatilityStructure::volatility(
                                                        const Date& start, 
                                                        Rate strike) const {
        Date settl = settlementDate();
        Time t = dayCounter().yearFraction(settl,start,settl,start);
        return volatilityImpl(t,strike);
    }

    inline Volatility CapletForwardVolatilityStructure::volatility(
                                                        Time t, 
                                                        Rate strike) const {
        return volatilityImpl(t,strike);
    }

}


#endif
