
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

#include <ql/basetermstructure.hpp>

namespace QuantLib {

    //! Cap/floor flat volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapFlatVolatilityStructure : public BaseTermStructure {
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
        CapFlatVolatilityStructure();
        #ifndef QL_DISABLE_DEPRECATED
        //! initialize with a fixed today and reference date
        CapFlatVolatilityStructure(const Date& today,
                                   const Date& referenceDate);
        #endif
        //! initialize with a fixed reference date
        CapFlatVolatilityStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        CapFlatVolatilityStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~CapFlatVolatilityStructure() {}
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


    //! Caplet/floorlet forward volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapletForwardVolatilityStructure : public BaseTermStructure {
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
        CapletForwardVolatilityStructure();
        #ifndef QL_DISABLE_DEPRECATED
        //! initialize with a fixed today and reference date
        CapletForwardVolatilityStructure(const Date& today,
                                         const Date& referenceDate);
        #endif
        //! initialize with a fixed reference date
        CapletForwardVolatilityStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        CapletForwardVolatilityStructure(Integer settlementDays,
                                         const Calendar&);
        //@}
        virtual ~CapletForwardVolatilityStructure() {}
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

    inline CapFlatVolatilityStructure::CapFlatVolatilityStructure() {}

    inline CapFlatVolatilityStructure::CapFlatVolatilityStructure(
                                                   const Date& referenceDate)
    : BaseTermStructure(referenceDate) {}

    #ifndef QL_DISABLE_DEPRECATED
    inline CapFlatVolatilityStructure::CapFlatVolatilityStructure(
                                 const Date& today, const Date& referenceDate)
    : BaseTermStructure(today,referenceDate) {}
    #endif

    inline CapFlatVolatilityStructure::CapFlatVolatilityStructure(
                             Integer settlementDays, const Calendar& calendar)
    : BaseTermStructure(settlementDays,calendar) {}

    inline Volatility CapFlatVolatilityStructure::volatility(const Date& end,
                                                             Rate strike)
                                                                     const {
        return volatilityImpl(timeFromReference(end),strike);
    }

    inline Volatility CapFlatVolatilityStructure::volatility(
                                                        const Period& length,
                                                        Rate strike) const {
        Date end = referenceDate().plus(length);
        return volatilityImpl(timeFromReference(end),strike);
    }

    inline Volatility CapFlatVolatilityStructure::volatility(Time t,
                                                             Rate strike)
                                                                     const {
        return volatilityImpl(t,strike);
    }



    inline
    CapletForwardVolatilityStructure::CapletForwardVolatilityStructure() {}

    inline
    CapletForwardVolatilityStructure::CapletForwardVolatilityStructure(
                                                   const Date& referenceDate)
    : BaseTermStructure(referenceDate) {}

    #ifndef QL_DISABLE_DEPRECATED
    inline
    CapletForwardVolatilityStructure::CapletForwardVolatilityStructure(
                                 const Date& today, const Date& referenceDate)
    : BaseTermStructure(today,referenceDate) {}
    #endif

    inline
    CapletForwardVolatilityStructure::CapletForwardVolatilityStructure(
                             Integer settlementDays, const Calendar& calendar)
    : BaseTermStructure(settlementDays,calendar) {}

    inline Volatility CapletForwardVolatilityStructure::volatility(
                                                        const Date& start,
                                                        Rate strike) const {
        return volatilityImpl(timeFromReference(start),strike);
    }

    inline Volatility CapletForwardVolatilityStructure::volatility(
                                                        Time t,
                                                        Rate strike) const {
        return volatilityImpl(t,strike);
    }

}


#endif
