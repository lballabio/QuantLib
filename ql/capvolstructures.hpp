
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

/*! \file capvolstructures.hpp
    \brief Cap/Floor volatility structures
*/

#ifndef quantlib_cap_volatility_structures_hpp
#define quantlib_cap_volatility_structures_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    //! Cap/floor term-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapVolatilityStructure : public TermStructure {
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
        CapVolatilityStructure();
        //! initialize with a fixed reference date
        CapVolatilityStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        CapVolatilityStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~CapVolatilityStructure() {}
        //! \name Volatility
        //@{
        Volatility volatility(const Date& end, Rate strike) const;
        //! returns the volatility for a given cap/floor length and strike rate
        Volatility volatility(const Period& length, Rate strike) const;
        //! returns the volatility for a given end time and strike rate
        Volatility volatility(Time t, Rate strike) const;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
    };

    //! Caplet/floorlet forward-volatility structure
    /*! This class is purely abstract and defines the interface of
        concrete structures which will be derived from this one.
    */
    class CapletVolatilityStructure : public TermStructure {
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
        CapletVolatilityStructure();
        //! initialize with a fixed reference date
        CapletVolatilityStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        CapletVolatilityStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~CapletVolatilityStructure() {}
        //! \name Volatility
        //@{
        //! returns the volatility for a given start date and strike rate
        Volatility volatility(const Date& start, Rate strike) const ;
        //! returns the volatility for a given start time and strike rate
        Volatility volatility(Time t, Rate strike) const;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
    };


    // inline definitions

    inline CapVolatilityStructure::CapVolatilityStructure() {}

    inline CapVolatilityStructure::CapVolatilityStructure(
                                                   const Date& referenceDate)
    : TermStructure(referenceDate) {}

    inline CapVolatilityStructure::CapVolatilityStructure(
                             Integer settlementDays, const Calendar& calendar)
    : TermStructure(settlementDays,calendar) {}

    inline Volatility CapVolatilityStructure::volatility(const Date& end,
                                                         Rate strike) const {
        return volatilityImpl(timeFromReference(end),strike);
    }

    inline Volatility CapVolatilityStructure::volatility(const Period& length,
                                                         Rate strike) const {
        Date end = referenceDate() + length;
        return volatilityImpl(timeFromReference(end),strike);
    }

    inline Volatility CapVolatilityStructure::volatility(Time t,
                                                         Rate strike) const {
        return volatilityImpl(t,strike);
    }



    inline CapletVolatilityStructure::CapletVolatilityStructure() {}

    inline CapletVolatilityStructure::CapletVolatilityStructure(
                                                   const Date& referenceDate)
    : TermStructure(referenceDate) {}

    inline CapletVolatilityStructure::CapletVolatilityStructure(
                             Integer settlementDays, const Calendar& calendar)
    : TermStructure(settlementDays,calendar) {}

    inline Volatility CapletVolatilityStructure::volatility(
                                                        const Date& start,
                                                        Rate strike) const {
        return volatilityImpl(timeFromReference(start),strike);
    }

    inline Volatility CapletVolatilityStructure::volatility(
                                                        Time t,
                                                        Rate strike) const {
        return volatilityImpl(t,strike);
    }

}


#endif
