
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file voltermstructure.hpp
    \brief Volatility term structure

    \fullpath
    ql/%voltermstructure.hpp
*/

// $Id$

#ifndef quantlib_vol_term_structure_hpp
#define quantlib_vol_term_structure_hpp

#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/marketelement.hpp>
#include <vector>

/*! \namespace QuantLib::VolTermStructures
    \brief Concrete implementations of the VolTermStructure interface

*/

namespace QuantLib {

    //! Volatility Term structure
    /*! This abstract class defines the interface of concrete
        volatility term structures which will be derived from this one.

        Volatility are assumed to be expressed on an annual basis
    */
    class VolTermStructure : public Patterns::Observable {
      public:
        virtual ~VolTermStructure() {}
        //! \name Black Volatility
        //@{
        //! Black present (a.k.a spot) volatility
        double blackVol(const Date& maturity,
                        double strike,
                        bool extrapolate = false) const;
        //! Black present (a.k.a spot) volatility
        double blackVol(Time maturity,
                        double strike,
                        bool extrapolate = false) const;
        //! Black present (a.k.a spot) variance
        double blackVariance(const Date& maturity,
                             double strike,
                             bool extrapolate = false) const;
        //! Black present (a.k.a spot) variance
        double blackVariance(Time maturity,
                             double strike,
                             bool extrapolate = false) const;
        //! Black future (a.k.a. forward) volatility
        double blackForwardVol(const Date& date1,
                               const Date& date2,
                               double strike,
                               bool extrapolate = false) const;
        //! Black future (a.k.a. forward) volatility
        double blackForwardVol(Time time1,
                               Time time2,
                               double strike,
                               bool extrapolate = false) const;
        //! Black future (a.k.a. forward) variance
        double blackForwardVariance(const Date& date1,
                                    const Date& date2,
                                    double strike,
                                    bool extrapolate = false) const;
        //! Black future (a.k.a. forward) variance
        double blackForwardVariance(Time time1,
                                    Time time2,
                                    double strike,
                                    bool extrapolate = false) const;
        //@}
        //! \name Dates
        //@{
        //! returns the reference date for which t=0
        virtual Date referenceDate() const = 0;
        //! returns the day counter
        virtual DayCounter dayCounter() const = 0;
        //! the latest date for which the term structure can return vols
        virtual Date maxDate() const = 0;
        //! the latest time for which the term structure can return vols
        Time maxTime() const;
        //@}
      protected:
        //! implements the actual Black variance calculation in derived classes
        virtual double blackVarianceImpl(Time t, double strike,
            bool extrapolate = false) const = 0;
        //! implements the actual Black vol calculation in derived classes
        virtual double blackVolImpl(Time t, double strike,
            bool extrapolate = false) const = 0;
    };


    inline double VolTermStructure::maxTime() const {
        return dayCounter().yearFraction(referenceDate(), maxDate());
    }

    inline double VolTermStructure::blackVol(const Date& maturity,
        double strike, bool extrapolate) const {

        Time t = dayCounter().yearFraction(referenceDate(), maturity);
        return blackVolImpl(t, strike, extrapolate);
    }

    inline double VolTermStructure::blackVol(Time maturity,
        double strike, bool extrapolate) const {

        return blackVolImpl(maturity, strike, extrapolate);
    }

    inline double VolTermStructure::blackVariance(const Date& maturity,
        double strike, bool extrapolate) const {

        Time t = dayCounter().yearFraction(referenceDate(), maturity);
        return blackVarianceImpl(t, strike, extrapolate);
    }

    inline double VolTermStructure::blackVariance(Time maturity,
        double strike, bool extrapolate) const {

        return blackVarianceImpl(maturity, strike, extrapolate);
    }


    inline double VolTermStructure::blackForwardVol(const Date& date1,
        const Date& date2, double strike, bool extrapolate) const {

        Time time1 = dayCounter().yearFraction(referenceDate(), date1);
        Time time2 = dayCounter().yearFraction(referenceDate(), date2);
        return blackForwardVol(time1, time2, strike, extrapolate);
    }


    inline double VolTermStructure::blackForwardVol(Time time1, Time time2,
        double strike, bool extrapolate) const {

        QL_REQUIRE(time2>=time1,
            "VolTermStructure::blackForwardVol : "
            "time2<time1");
        double var1 = blackVarianceImpl(time1, strike, extrapolate);
        if (time2==time1) {
            Time epsilon = 0.00001;
            return QL_SQRT(
                (blackVarianceImpl(time1+epsilon, strike, extrapolate)-var1)
                /epsilon
                );
        } else {
            return QL_SQRT(
                (blackVarianceImpl(time2, strike, extrapolate)-var1)
                /(time2-time1)
                );
        }
    }


    inline double VolTermStructure::blackForwardVariance(const Date& date1,
        const Date& date2, double strike, bool extrapolate) const {

        Time time1 = dayCounter().yearFraction(referenceDate(), date1);
        Time time2 = dayCounter().yearFraction(referenceDate(), date2);
        return blackForwardVariance(time1, time2, strike, extrapolate);
    }


    inline double VolTermStructure::blackForwardVariance(Time time1,
        Time time2, double strike, bool extrapolate) const {

        QL_REQUIRE(time2>time1,
            "VolTermStructure::blackForwardVariance : "
            "time2<=time1");
        double v1 = blackVarianceImpl(time1, strike, extrapolate);
        double v2 = blackVarianceImpl(time2, strike, extrapolate);
        return (v2-v1);
    }

    //! Volatility term structure
    /*! This abstract class acts as an adapter to VolTermStructure allowing the
        programmer to implement only the
        <tt>blackVolImpl(Time, double, bool)</tt> method in derived classes.

        Volatility are assumed to be expressed on an annual basis
    */
    class VolatilityTermStructure : public VolTermStructure {
      public:
        virtual ~VolatilityTermStructure() {}
      protected:
        /*! Returns the variance for the given strike and date calculating it
            from the volatility.
        */
        double blackVarianceImpl(Time maturity, double strike,
            bool extrapolate = false) const;
    };

    inline double VolatilityTermStructure ::blackVarianceImpl(Time maturity,
        double strike, bool extrapolate) const {

        double vol = blackVolImpl(maturity, strike, extrapolate);
        return vol*vol*maturity;
    }

    //! Variance term structure
    /*! This abstract class acts as an adapter to VolTermStructure allowing the
        programmer to implement only the
        <tt>blackVarianceImpl(Time, double, bool)</tt> method in derived
        classes.

        Volatility are assumed to be expressed on an annual basis
    */
    class VarianceTermStructure : public VolTermStructure {
      public:
        virtual ~VarianceTermStructure() {}
      protected:
        /*! Returns the volatility for the given strike and date calculating it
            from the variance.
        */
        double blackVolImpl(Time maturity, double strike,
            bool extrapolate = false) const;
    };

    inline double VarianceTermStructure ::blackVolImpl(Time maturity,
        double strike, bool extrapolate) const {

        Time nonZeroMaturity = (maturity==0.0 ? 0.00001 : maturity);
        double var = blackVarianceImpl(nonZeroMaturity, strike, extrapolate);
        return QL_SQRT(var/nonZeroMaturity);
    }

}


#endif
