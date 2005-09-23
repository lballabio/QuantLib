/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

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

/*! \file capvolstructures.hpp
    \brief Cap/Floor volatility structures
*/

#ifndef quantlib_cap_volatility_structures_hpp
#define quantlib_cap_volatility_structures_hpp

#include <ql/termstructure.hpp>
#include <ql/Math/extrapolation.hpp>

namespace QuantLib {

    //! Cap/floor term-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapVolatilityStructure : public TermStructure,
                                   public Extrapolator {
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
        Volatility volatility(const Date& end, Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given cap/floor length and strike rate
        Volatility volatility(const Period& length, Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given end time and strike rate
        Volatility volatility(Time t, Rate strike,
                              bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the latest date for which the term structure can return vols
        virtual Date maxDate() const = 0;
        //! the latest time for which the term structure can return vols
        virtual Time maxTime() const;
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
	  private:
        void checkRange(Time, Real strike, bool extrapolate) const;
    };

    //! Caplet/floorlet forward-volatility structure
    /*! This class is purely abstract and defines the interface of
        concrete structures which will be derived from this one.
    */
    class CapletVolatilityStructure : public TermStructure,
                                      public Extrapolator {
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
        Volatility volatility(const Date& start, Rate strike,
                              bool extrapolate = false) const ;
        //! returns the volatility for a given start time and strike rate
        Volatility volatility(Time t, Rate strike,
                              bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the latest date for which the term structure can return vols
        virtual Date maxDate() const = 0;
        //! the latest time for which the term structure can return vols
        virtual Time maxTime() const;
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
	  private:
        void checkRange(Time, Real strike, bool extrapolate) const;
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
                                                         Rate strike,
                                                         bool extrapolate)
                                                                       const {
        Time t = timeFromReference(end);
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility CapVolatilityStructure::volatility(const Period& length,
                                                         Rate strike,
                                                         bool extrapolate)
                                                                       const {
        Date end = referenceDate() + length;
        Time t = timeFromReference(end);
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility CapVolatilityStructure::volatility(Time t,
                                                         Rate strike,
                                                         bool extrapolate)
                                                                       const {
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

	inline Time CapVolatilityStructure::maxTime() const {
        return timeFromReference(maxDate());
    }

    inline void CapVolatilityStructure::checkRange(
                                     Time t, Rate k, bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
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
                                                        Rate strike,
                                                        bool extrapolate)
                                                                       const {
        Time t = timeFromReference(start);
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility CapletVolatilityStructure::volatility(
                                                        Time t,
                                                        Rate strike,
                                                        bool extrapolate)
                                                                       const {
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

	inline Time CapletVolatilityStructure::maxTime() const {
        return timeFromReference(maxDate());
    }

    inline void CapletVolatilityStructure::checkRange(
                                     Time t, Rate k, bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}


#endif
