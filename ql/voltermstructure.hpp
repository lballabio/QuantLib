
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file voltermstructure.hpp
    \brief Volatility term structures
*/

#ifndef quantlib_vol_term_structure_hpp
#define quantlib_vol_term_structure_hpp

#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/marketelement.hpp>
#include <ql/Patterns/visitor.hpp>
#include <vector>

namespace QuantLib {

    //! Black Volatility Term structure
    /*! This abstract class defines the interface of concrete
        Black volatility term structures which will be derived from this one.

        Volatilities are assumed to be expressed on an annual basis
    */
    class BlackVolTermStructure : public Observable {
      public:
        virtual ~BlackVolTermStructure() {}
        //! \name Black Volatility
        //@{
        //! present (a.k.a spot) volatility
        double blackVol(const Date& maturity,
                        double strike,
                        bool extrapolate = false) const;
        //! present (a.k.a spot) volatility
        double blackVol(Time maturity,
                        double strike,
                        bool extrapolate = false) const;
        //! present (a.k.a spot) variance
        double blackVariance(const Date& maturity,
                             double strike,
                             bool extrapolate = false) const;
        //! present (a.k.a spot) variance
        double blackVariance(Time maturity,
                             double strike,
                             bool extrapolate = false) const;
        //! future (a.k.a. forward) volatility
        double blackForwardVol(const Date& date1,
                               const Date& date2,
                               double strike,
                               bool extrapolate = false) const;
        //! future (a.k.a. forward) volatility
        double blackForwardVol(Time time1,
                               Time time2,
                               double strike,
                               bool extrapolate = false) const;
        //! future (a.k.a. forward) variance
        double blackForwardVariance(const Date& date1,
                                    const Date& date2,
                                    double strike,
                                    bool extrapolate = false) const;
        //! future (a.k.a. forward) variance
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
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        //! implements the actual Black variance calculation in derived classes
        virtual double blackVarianceImpl(Time t,
                                         double strike,
                                         bool extrapolate = false) const = 0;
        //! implements the actual Black vol calculation in derived classes
        virtual double blackVolImpl(Time t,
                                    double strike,
                                    bool extrapolate = false) const = 0;
	  private:
        static const double dT;
    };

   

    //! Black Volatility term structure
    /*! This abstract class acts as an adapter to BlackVolTermStructure 
        allowing the programmer to implement only the
        <tt>blackVolImpl(Time, double, bool)</tt> method in derived classes.

        Volatility are assumed to be expressed on an annual basis.
    */
    class BlackVolatilityTermStructure : public BlackVolTermStructure {
      public:
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! Returns the variance for the given strike and date calculating it
            from the volatility.
        */
        double blackVarianceImpl(Time maturity,
                                 double strike,
                                 bool extrapolate = false) const;
    };


    //! Black Variance term structure
    /*! This abstract class acts as an adapter to VolTermStructure allowing 
        the programmer to implement only the
        <tt>blackVarianceImpl(Time, double, bool)</tt> method in derived
        classes.

        Volatility are assumed to be expressed on an annual basis.
    */
    class BlackVarianceTermStructure : public BlackVolTermStructure {
      public:
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! Returns the volatility for the given strike and date calculating it
            from the variance.
        */
        double blackVolImpl(Time maturity,
                            double strike,
                            bool extrapolate = false) const;
    };


    //! Local Volatility Term structure
    /*! This abstract class defines the interface of concrete
        local volatility term structures which will be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class LocalVolTermStructure : public Observable {
      public:
        virtual ~LocalVolTermStructure() {}
        //! \name Local Volatility
        //@{
        double localVol(const Date& d,
                        double underlyingLevel,
                        bool extrapolate = false) const;
        double localVol(Time t,
                        double underlyingLevel,
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
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        //! implements the actual local vol calculation in derived classes
        virtual double localVolImpl(Time t,
                                    double strike,
                                    bool extrapolate = false) const = 0;
    };



    // inline definitions

    inline void BlackVolTermStructure::accept(AcyclicVisitor& v) {
        Visitor<BlackVolTermStructure>* v1 = 
            dynamic_cast<Visitor<BlackVolTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("Not a Black-volatility term structure visitor");
    }

    inline void BlackVolatilityTermStructure::accept(AcyclicVisitor& v) {
        Visitor<BlackVolatilityTermStructure>* v1 = 
            dynamic_cast<Visitor<BlackVolatilityTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVolTermStructure::accept(v);
    }

    inline void BlackVarianceTermStructure::accept(AcyclicVisitor& v) {
        Visitor<BlackVarianceTermStructure>* v1 = 
            dynamic_cast<Visitor<BlackVarianceTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVolTermStructure::accept(v);
    }

    inline void LocalVolTermStructure::accept(AcyclicVisitor& v) {
        Visitor<LocalVolTermStructure>* v1 = 
            dynamic_cast<Visitor<LocalVolTermStructure>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("Not a local-volatility term structure visitor");
    }

}


#endif
