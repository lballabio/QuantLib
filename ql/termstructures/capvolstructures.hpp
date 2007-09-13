/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file capvolstructures.hpp
    \brief cap/floor volatility structures
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
        CapVolatilityStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        CapVolatilityStructure(const Date& referenceDate,
                               const Calendar& cal = Calendar(),
                               const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        CapVolatilityStructure(Natural settlementDays,
                               const Calendar&,
                               const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~CapVolatilityStructure() {}
        //! \name Volatility
        //@{
        Volatility volatility(const Date& end,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given cap/floor length and strike rate
        Volatility volatility(const Period& length,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given end time and strike rate
        Volatility volatility(Time t,
                              Rate strike,
                              bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
      private:
        void checkRange(Time, Rate strike, bool extrapolate) const;
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
        CapletVolatilityStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        CapletVolatilityStructure(const Date& referenceDate,
                                  const Calendar& cal = Calendar(),
                                  const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        CapletVolatilityStructure(Natural settlementDays,
                                  const Calendar&,
                                  const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~CapletVolatilityStructure() {}
        //! \name Volatility and Variance
        //@{
        //! returns the volatility for a given exercise date and strike rate
        Volatility volatility(const Date& exerciseDate,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given exercise time and strike rate
        Volatility volatility(Time t,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option tenor and strike rate
        Volatility volatility(const Period& optionTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given exercise date and strike rate
        Real blackVariance(const Date& exerciseDate,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given start time and strike rate
        Real blackVariance(Time t,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given option tenor and strike rate
        Volatility blackVariance(const Period& optionTenor,
                                 Rate strike,
                                 bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length,
                                          Rate strike) const = 0;
      private:
        void checkRange(Time, Rate strike, bool extrapolate) const;
    };


    // inline CapVolatilityStructure definitions

    inline CapVolatilityStructure::CapVolatilityStructure(const DayCounter& dc)
    : TermStructure(dc) {}

    inline
    CapVolatilityStructure::CapVolatilityStructure(const Date& refDate,
                                                   const Calendar& cal,
                                                   const DayCounter& dc)
    : TermStructure(refDate, cal, dc) {}

    inline
    CapVolatilityStructure::CapVolatilityStructure(Natural settlementDays,
                                                   const Calendar& cal,
                                                   const DayCounter& dc)
    : TermStructure(settlementDays, cal, dc) {}


    inline
    Volatility CapVolatilityStructure::volatility(const Date& end,
                                                  Rate strike,
                                                  bool extrapolate) const {
        Time t = timeFromReference(end);
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline
    Volatility CapVolatilityStructure::volatility(Time t,
                                                  Rate strike,
                                                  bool extrapolate) const {
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline
    Volatility CapVolatilityStructure::volatility(const Period& optionTenor,
                                                  Rate strike,
                                                  bool extrapolate) const {
        Date exerciseDate = calendar().advance(referenceDate(),
                                               optionTenor,
                                               Following); //FIXME
        return volatility(exerciseDate, strike, extrapolate);
    }

    inline
    void CapVolatilityStructure::checkRange(Time t,
                                            Rate k,
                                            bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }


    // inline CapletVolatilityStructure definitions

    inline
    CapletVolatilityStructure::CapletVolatilityStructure(const DayCounter& dc)
    : TermStructure(dc) {}

    inline CapletVolatilityStructure::CapletVolatilityStructure(
                                                   const Date& referenceDate,
                                                   const Calendar& cal,
                                                   const DayCounter& dc)
    : TermStructure(referenceDate, cal, dc) {}

    inline CapletVolatilityStructure::CapletVolatilityStructure(
                             Natural settlementDays,
                             const Calendar& cal,
                             const DayCounter& dc)
    : TermStructure(settlementDays, cal, dc) {}

    inline
    Volatility CapletVolatilityStructure::volatility(const Date& start,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Time t = timeFromReference(start);
        checkRange(t, strike, extrapolate);
        return volatilityImpl(t,strike);
    }

    inline
    Volatility CapletVolatilityStructure::volatility(Time t,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(t, strike, extrapolate);
        return volatilityImpl(t,strike);
    }

    inline
    Volatility CapletVolatilityStructure::volatility(const Period& optionTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Date exerciseDate = calendar().advance(referenceDate(),
                                               optionTenor,
                                               Following); //FIXME
        return volatility(exerciseDate, strike, extrapolate);
    }

    inline
    Volatility CapletVolatilityStructure::blackVariance(const Date& start,
                                                        Rate strike,
                                                        bool extrap) const {
        Time t = timeFromReference(start);
        checkRange(t, strike, extrap);
        Volatility vol = volatilityImpl(t, strike);
        return vol*vol*t;
    }

    inline
    Volatility CapletVolatilityStructure::blackVariance(Time t,
                                                        Rate strike,
                                                        bool extrap) const {
        checkRange(t, strike, extrap);
        Volatility vol = volatilityImpl(t, strike);
        return vol*vol*t;
    }

    inline
    Volatility CapletVolatilityStructure::blackVariance(const Period& optionT,
                                                        Rate strike,
                                                        bool extrap) const {
        Date exerciseDate = calendar().advance(referenceDate(),
                                               optionT,
                                               Following); //FIXME
        return blackVariance(exerciseDate, strike, extrap);
    }

    inline void CapletVolatilityStructure::checkRange(Time t,
                                                      Rate k,
                                                      bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}

#endif
