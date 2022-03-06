/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

/*! \file blackvoltermstructure.hpp
    \brief Black volatility term structure base classes
*/

#ifndef quantlib_black_vol_term_structures_hpp
#define quantlib_black_vol_term_structures_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/patterns/visitor.hpp>

namespace QuantLib {

    //! Black-volatility term structure
    /*! This abstract class defines the interface of concrete
        Black-volatility term structures which will be derived from
        this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class BlackVolTermStructure : public VolatilityTermStructure {
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
        BlackVolTermStructure(BusinessDayConvention bdc = Following,
                              const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        BlackVolTermStructure(const Date& referenceDate,
                              const Calendar& cal = Calendar(),
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        BlackVolTermStructure(Natural settlementDays,
                              const Calendar&,
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = DayCounter());
        //@}
        ~BlackVolTermStructure() override = default;
        //! \name Black Volatility
        //@{
        //! spot volatility
        Volatility blackVol(const Date& maturity,
                            Real strike,
                            bool extrapolate = false) const;
        //! spot volatility
        Volatility blackVol(Time maturity,
                            Real strike,
                            bool extrapolate = false) const;
        //! spot variance
        Real blackVariance(const Date& maturity,
                           Real strike,
                           bool extrapolate = false) const;
        //! spot variance
        Real blackVariance(Time maturity,
                           Real strike,
                           bool extrapolate = false) const;
        //! forward (at-the-money) volatility
        Volatility blackForwardVol(const Date& date1,
                                   const Date& date2,
                                   Real strike,
                                   bool extrapolate = false) const;
        //! forward (at-the-money) volatility
        Volatility blackForwardVol(Time time1,
                                   Time time2,
                                   Real strike,
                                   bool extrapolate = false) const;
        //! forward (at-the-money) variance
        Real blackForwardVariance(const Date& date1,
                                  const Date& date2,
                                  Real strike,
                                  bool extrapolate = false) const;
        //! forward (at-the-money) variance
        Real blackForwardVariance(Time time1,
                                  Time time2,
                                  Real strike,
                                  bool extrapolate = false) const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! \name Calculations

            These methods must be implemented in derived classes to perform
            the actual volatility calculations. When they are called,
            range check has already been performed; therefore, they must
            assume that extrapolation is required.
        */
        //@{
        //! Black variance calculation
        virtual Real blackVarianceImpl(Time t, Real strike) const = 0;
        //! Black volatility calculation
        virtual Volatility blackVolImpl(Time t, Real strike) const = 0;
        //@}
    };

    //! Black-volatility term structure
    /*! This abstract class acts as an adapter to BlackVolTermStructure
        allowing the programmer to implement only the
        <tt>blackVolImpl(Time, Real, bool)</tt> method in derived classes.

        Volatility are assumed to be expressed on an annual basis.
    */
    class BlackVolatilityTermStructure : public BlackVolTermStructure {
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
        BlackVolatilityTermStructure(BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        BlackVolatilityTermStructure(const Date& referenceDate,
                                     const Calendar& cal = Calendar(),
                                     BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        BlackVolatilityTermStructure(Natural settlementDays,
                                     const Calendar& cal,
                                     BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = DayCounter());
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        /*! Returns the variance for the given strike and date calculating it
            from the volatility.
        */
        Real blackVarianceImpl(Time maturity, Real strike) const override;
    };


    //! Black variance term structure
    /*! This abstract class acts as an adapter to VolTermStructure allowing
        the programmer to implement only the
        <tt>blackVarianceImpl(Time, Real, bool)</tt> method in derived
        classes.

        Volatility are assumed to be expressed on an annual basis.
    */
    class BlackVarianceTermStructure : public BlackVolTermStructure {
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
        BlackVarianceTermStructure(BusinessDayConvention bdc = Following,
                                   const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        BlackVarianceTermStructure(const Date& referenceDate,
                                   const Calendar& cal = Calendar(),
                                   BusinessDayConvention bdc = Following,
                                   const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        BlackVarianceTermStructure(Natural settlementDays,
                                   const Calendar&,
                                   BusinessDayConvention bdc = Following,
                                   const DayCounter& dc = DayCounter());
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        /*! Returns the volatility for the given strike and date calculating it
            from the variance.
        */
        Volatility blackVolImpl(Time t, Real strike) const override;
    };



    // inline definitions

    inline Volatility BlackVolTermStructure::blackVol(const Date& d,
                                                      Real strike,
                                                      bool extrapolate) const {
        checkRange(d, extrapolate);
        checkStrike(strike, extrapolate);
        Time t = timeFromReference(d);
        return blackVolImpl(t, strike);
    }

    inline Volatility BlackVolTermStructure::blackVol(Time t,
                                                      Real strike,
                                                      bool extrapolate) const {
        checkRange(t, extrapolate);
        checkStrike(strike, extrapolate);
        return blackVolImpl(t, strike);
    }

    inline Real BlackVolTermStructure::blackVariance(const Date& d,
                                                     Real strike,
                                                     bool extrapolate) const {
        checkRange(d, extrapolate);
        checkStrike(strike, extrapolate);
        Time t = timeFromReference(d);
        return blackVarianceImpl(t, strike);
    }

    inline Real BlackVolTermStructure::blackVariance(Time t,
                                                     Real strike,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);
        checkStrike(strike, extrapolate);
        return blackVarianceImpl(t, strike);
    }

    inline void BlackVolTermStructure::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackVolTermStructure>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            QL_FAIL("not a Black-volatility term structure visitor");
    }

    inline
    Real BlackVolatilityTermStructure::blackVarianceImpl(Time t,
                                                         Real strike) const {
        Volatility vol = blackVolImpl(t, strike);
        return vol*vol*t;
    }

    inline void BlackVolatilityTermStructure::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackVolatilityTermStructure>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVolTermStructure::accept(v);
    }

    inline
    Volatility BlackVarianceTermStructure ::blackVolImpl(Time t,
                                                         Real strike) const {
        Time nonZeroMaturity = (t==0.0 ? 0.00001 : t);
        Real var = blackVarianceImpl(nonZeroMaturity, strike);
        return std::sqrt(var/nonZeroMaturity);
    }

    inline void BlackVarianceTermStructure::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackVarianceTermStructure>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVolTermStructure::accept(v);
    }

}

#endif


#ifndef id_5f08b4e1b60093c093e34b511b763ced
#define id_5f08b4e1b60093c093e34b511b763ced
inline bool test_5f08b4e1b60093c093e34b511b763ced(int* i) { return i != 0; }
#endif
