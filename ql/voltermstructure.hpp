/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

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
#include <ql/quote.hpp>
#include <ql/Math/extrapolation.hpp>
#include <ql/Patterns/visitor.hpp>
#include <vector>

namespace QuantLib {

    //! Black-volatility term structure
    /*! This abstract class defines the interface of concrete
        Black-volatility term structures which will be derived from
        this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class BlackVolTermStructure : public TermStructure,
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
        BlackVolTermStructure();
        //! initialize with a fixed reference date
        BlackVolTermStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        BlackVolTermStructure(Integer settlementDays, const Calendar&);
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
        //! the latest date for which the term structure can return vols
        virtual Date maxDate() const = 0;
        //! the latest time for which the term structure can return vols
        Time maxTime() const;
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
        void checkRange(const Date&, Real strike, bool extrapolate) const;
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
        BlackVolatilityTermStructure();
        //! initialize with a fixed reference date
        BlackVolatilityTermStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        BlackVolatilityTermStructure(Integer settlementDays, const Calendar&);
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
        BlackVarianceTermStructure();
        //! initialize with a fixed reference date
        BlackVarianceTermStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        BlackVarianceTermStructure(Integer settlementDays, const Calendar&);
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! Returns the volatility for the given strike and date calculating it
            from the variance.
        */
        Volatility blackVolImpl(Time maturity, Real strike) const;
    };


    //! Local-volatility term structure
    /*! This abstract class defines the interface of concrete
        local-volatility term structures which will be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class LocalVolTermStructure : public TermStructure,
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
        LocalVolTermStructure();
        //! initialize with a fixed reference date
        LocalVolTermStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        LocalVolTermStructure(Integer settlementDays, const Calendar&);
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
        //! the latest date for which the term structure can return vols
        virtual Date maxDate() const = 0;
        //! the latest time for which the term structure can return vols
        Time maxTime() const;
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
        void checkRange(const Date&, Real strike, bool extrapolate) const;
        void checkRange(Time, Real strike, bool extrapolate) const;
    };



    // inline definitions

    inline BlackVolTermStructure::BlackVolTermStructure() {}

    inline BlackVolTermStructure::BlackVolTermStructure(
                                                   const Date& referenceDate)
    : TermStructure(referenceDate) {}

    inline BlackVolTermStructure::BlackVolTermStructure(
                             Integer settlementDays, const Calendar& calendar)
    : TermStructure(settlementDays,calendar) {}

	inline Time BlackVolTermStructure::maxTime() const {
        return timeFromReference(maxDate());
    }

    inline Volatility BlackVolTermStructure::blackVol(const Date& maturity,
                                                      Real strike,
                                                      bool extrapolate) const {
        checkRange(maturity,strike,extrapolate);
        Time t = timeFromReference(maturity);
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
        checkRange(maturity,strike,extrapolate);
        Time t = timeFromReference(maturity);
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

    inline void BlackVolTermStructure::checkRange(const Date& d, Real k,
                                                  bool extrapolate) const {
        Time t = timeFromReference(d);
        checkRange(t,k,extrapolate);
    }

    inline void BlackVolTermStructure::checkRange(Time t, Real k,
                                                  bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }



    inline BlackVolatilityTermStructure::BlackVolatilityTermStructure() {}

    inline BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                                                   const Date& referenceDate)
    : BlackVolTermStructure(referenceDate) {}

    inline BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                             Integer settlementDays, const Calendar& calendar)
    : BlackVolTermStructure(settlementDays,calendar) {}

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



    inline BlackVarianceTermStructure::BlackVarianceTermStructure() {}

    inline BlackVarianceTermStructure::BlackVarianceTermStructure(
                                                   const Date& referenceDate)
    : BlackVolTermStructure(referenceDate) {}

    inline BlackVarianceTermStructure::BlackVarianceTermStructure(
                             Integer settlementDays, const Calendar& calendar)
    : BlackVolTermStructure(settlementDays,calendar) {}

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



    inline LocalVolTermStructure::LocalVolTermStructure() {}

    inline LocalVolTermStructure::LocalVolTermStructure(
                                                   const Date& referenceDate)
    : TermStructure(referenceDate) {}

    inline LocalVolTermStructure::LocalVolTermStructure(
                             Integer settlementDays, const Calendar& calendar)
    : TermStructure(settlementDays,calendar) {}

    inline Time LocalVolTermStructure::maxTime() const {
        return timeFromReference(maxDate());
    }

    inline Volatility LocalVolTermStructure::localVol(const Date& d,
                                                      Real underlyingLevel,
                                                      bool extrapolate) const {
        checkRange(d,underlyingLevel,extrapolate);
        Time t = timeFromReference(d);
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

    inline void LocalVolTermStructure::checkRange(const Date& d, Real k,
                                                  bool extrapolate) const {
        Time t = timeFromReference(d);
        checkRange(t,k,extrapolate);
    }

    inline void LocalVolTermStructure::checkRange(Time t, Real k,
                                                  bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike() << "]");
    }

}


#endif
