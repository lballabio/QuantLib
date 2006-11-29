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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file voltermstructure.hpp
    \brief Volatility term structures
*/

#ifndef quantlib_vol_term_structures_hpp
#define quantlib_vol_term_structures_hpp

#include <ql/termstructure.hpp>
#include <ql/Patterns/visitor.hpp>

namespace QuantLib {

    //! Black-volatility term structure
    /*! This abstract class defines the interface of concrete
        Black-volatility term structures which will be derived from
        this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class BlackVolTermStructure : public TermStructure {
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
        BlackVolTermStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        BlackVolTermStructure(const Date& referenceDate,
                              const Calendar& cal = Calendar(),
                              const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        BlackVolTermStructure(Integer settlementDays,
                              const Calendar&,
                              const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~BlackVolTermStructure() {}
        //! \name Black Volatility
        //@{
        //! present (a.k.a spot) volatility
        Volatility blackVol(const Date& maturity,
                            Real strike,
                            bool extrapolate = false) const;
        //! present (a.k.a spot) volatility
        Volatility blackVol(Time maturity,
                            Real strike,
                            bool extrapolate = false) const;
        //! present (a.k.a spot) variance
        Real blackVariance(const Date& maturity,
                           Real strike,
                           bool extrapolate = false) const;
        //! present (a.k.a spot) variance
        Real blackVariance(Time maturity,
                           Real strike,
                           bool extrapolate = false) const;
        //! future (a.k.a. forward) volatility
        Volatility blackForwardVol(const Date& date1,
                                   const Date& date2,
                                   Real strike,
                                   bool extrapolate = false) const;
        //! future (a.k.a. forward) volatility
        Volatility blackForwardVol(Time time1,
                                   Time time2,
                                   Real strike,
                                   bool extrapolate = false) const;
        //! future (a.k.a. forward) variance
        Real blackForwardVariance(const Date& date1,
                                  const Date& date2,
                                  Real strike,
                                  bool extrapolate = false) const;
        //! future (a.k.a. forward) variance
        Real blackForwardVariance(Time time1,
                                  Time time2,
                                  Real strike,
                                  bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
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
      private:
        static const Time dT;
        void checkRange(Time, Real strike, bool extrapolate) const;
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
        BlackVolatilityTermStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        BlackVolatilityTermStructure(const Date& referenceDate,
                                     const Calendar& cal = Calendar(),
                                     const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        BlackVolatilityTermStructure(Integer settlementDays,
                                     const Calendar&,
                                     const DayCounter& dc = Actual365Fixed());
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! Returns the variance for the given strike and date calculating it
            from the volatility.
        */
        Real blackVarianceImpl(Time maturity, Real strike) const;
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
        BlackVarianceTermStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        BlackVarianceTermStructure(const Date& referenceDate,
                                   const Calendar& cal = Calendar(),
                                   const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        BlackVarianceTermStructure(Integer settlementDays,
                                   const Calendar&,
                                   const DayCounter& dc = Actual365Fixed());
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! Returns the volatility for the given strike and date calculating it
            from the variance.
        */
        Volatility blackVolImpl(Time maturity,
                                Real strike) const;
    };


    //! Local-volatility term structure
    /*! This abstract class defines the interface of concrete
        local-volatility term structures which will be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class LocalVolTermStructure : public TermStructure {
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
        LocalVolTermStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        LocalVolTermStructure(const Date& referenceDate,
                              const Calendar& cal = Calendar(),
                              const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        LocalVolTermStructure(Integer settlementDays,
                              const Calendar&,
                              const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~LocalVolTermStructure() {}
        //! \name Local Volatility
        //@{
        Volatility localVol(const Date& d,
                            Real underlyingLevel,
                            bool extrapolate = false) const;
        Volatility localVol(Time t,
                            Real underlyingLevel,
                            bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
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
        //! local vol calculation
        virtual Volatility localVolImpl(Time t, Real strike) const = 0;
        //@}
      private:
        void checkRange(Time, Real strike, bool extrapolate) const;
    };



    // inline definitions

    inline BlackVolTermStructure::BlackVolTermStructure(const DayCounter& dc)
    : TermStructure(dc) {}

    inline BlackVolTermStructure::BlackVolTermStructure(const Date& refDate,
                                                        const Calendar& cal,
                                                        const DayCounter& dc)
    : TermStructure(refDate, cal, dc) {}

    inline BlackVolTermStructure::BlackVolTermStructure(Integer settlDays,
                                                        const Calendar& cal,
                                                        const DayCounter& dc)
    : TermStructure(settlDays, cal, dc) {}

    inline Volatility BlackVolTermStructure::blackVol(const Date& maturity,
                                                      Real strike,
                                                      bool extrapolate) const {
        Time t = timeFromReference(maturity);
        checkRange(t,strike,extrapolate);
        return blackVolImpl(t, strike);
    }

    inline Volatility BlackVolTermStructure::blackVol(Time maturity,
                                                      Real strike,
                                                      bool extrapolate) const {
        checkRange(maturity,strike,extrapolate);
        return blackVolImpl(maturity, strike);
    }

    inline Real BlackVolTermStructure::blackVariance(const Date& maturity,
                                                     Real strike,
                                                     bool extrapolate)
                                                                      const {
        Time t = timeFromReference(maturity);
        checkRange(t,strike,extrapolate);
        return blackVarianceImpl(t, strike);
    }

    inline Real BlackVolTermStructure::blackVariance(Time maturity,
                                                     Real strike,
                                                     bool extrapolate) const {
        checkRange(maturity,strike,extrapolate);
        return blackVarianceImpl(maturity, strike);
    }

    inline void BlackVolTermStructure::accept(AcyclicVisitor& v) {
        Visitor<BlackVolTermStructure>* v1 =
            dynamic_cast<Visitor<BlackVolTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not a Black-volatility term structure visitor");
    }

    inline void BlackVolTermStructure::checkRange(Time t, Real k,
                                                  bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }



    inline BlackVolatilityTermStructure::BlackVolatilityTermStructure(const DayCounter& dc)
    : BlackVolTermStructure(dc) {}

    inline BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                                                        const Date& refDate,
                                                        const Calendar& cal,
                                                        const DayCounter& dc)
    : BlackVolTermStructure(refDate, cal, dc) {}

    inline BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                             Integer settlementDays,
                             const Calendar& cal,
                             const DayCounter& dc)
    : BlackVolTermStructure(settlementDays, cal, dc) {}

    inline Real BlackVolatilityTermStructure::blackVarianceImpl(
                                         Time maturity, Real strike) const {
        Volatility vol = blackVolImpl(maturity, strike);
        return vol*vol*maturity;
    }

    inline void BlackVolatilityTermStructure::accept(AcyclicVisitor& v) {
        Visitor<BlackVolatilityTermStructure>* v1 =
            dynamic_cast<Visitor<BlackVolatilityTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVolTermStructure::accept(v);
    }



    inline BlackVarianceTermStructure::BlackVarianceTermStructure(const DayCounter& dc)
    : BlackVolTermStructure(dc) {}

    inline BlackVarianceTermStructure::BlackVarianceTermStructure(
                                                   const Date& referenceDate,
                                                   const Calendar& cal,
                                                   const DayCounter& dc)
    : BlackVolTermStructure(referenceDate, cal, dc) {}

    inline BlackVarianceTermStructure::BlackVarianceTermStructure(
                             Integer settlementDays,
                             const Calendar& cal,
                             const DayCounter& dc)
    : BlackVolTermStructure(settlementDays, cal, dc) {}

    inline Volatility BlackVarianceTermStructure ::blackVolImpl(
                                         Time maturity, Real strike) const {
        Time nonZeroMaturity = (maturity==0.0 ? 0.00001 : maturity);
        Real var = blackVarianceImpl(nonZeroMaturity, strike);
        return std::sqrt(var/nonZeroMaturity);
    }

    inline void BlackVarianceTermStructure::accept(AcyclicVisitor& v) {
        Visitor<BlackVarianceTermStructure>* v1 =
            dynamic_cast<Visitor<BlackVarianceTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVolTermStructure::accept(v);
    }



    inline LocalVolTermStructure::LocalVolTermStructure(const DayCounter& dc)
    : TermStructure(dc) {}

    inline LocalVolTermStructure::LocalVolTermStructure(
                                                   const Date& referenceDate,
                                                   const Calendar& cal,
                                                   const DayCounter& dc)
    : TermStructure(referenceDate, cal, dc) {}

    inline LocalVolTermStructure::LocalVolTermStructure(
                             Integer settlementDays,
                             const Calendar& cal,
                             const DayCounter& dc)
    : TermStructure(settlementDays, cal, dc) {}

    inline Volatility LocalVolTermStructure::localVol(const Date& d,
                                                      Real underlyingLevel,
                                                      bool extrapolate) const {
        Time t = timeFromReference(d);
        checkRange(t,underlyingLevel,extrapolate);
        return localVolImpl(t, underlyingLevel);
    }

    inline Volatility LocalVolTermStructure::localVol(Time t,
                                                      Real underlyingLevel,
                                                      bool extrapolate) const {
        checkRange(t,underlyingLevel,extrapolate);
        return localVolImpl(t, underlyingLevel);
    }

    inline void LocalVolTermStructure::accept(AcyclicVisitor& v) {
        Visitor<LocalVolTermStructure>* v1 =
            dynamic_cast<Visitor<LocalVolTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not a local-volatility term structure visitor");
    }

    inline void LocalVolTermStructure::checkRange(Time t, Real k,
                                                  bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike() << "]");
    }

}

#endif
