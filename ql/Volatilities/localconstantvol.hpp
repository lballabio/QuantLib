
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano

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

/*! \file localconstantvol.hpp
    \brief Local constant volatility, no time dependence, no asset dependence
*/

#ifndef quantlib_localconstantvol_hpp
#define quantlib_localconstantvol_hpp

#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    //! Constant local volatility, no time-strike dependence
    /*! This class implements the LocalVolatilityTermStructure
        interface for a constant local volatility (no time/asset
        dependence).  Local volatility and Black volatility are the
        same when volatility is at most time dependent, so this class
        is basically a proxy for BlackVolatilityTermStructure.
    */
    class LocalConstantVol : public LocalVolTermStructure {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        LocalConstantVol(const Date& referenceDate,
                         Volatility volatility,
                         const DayCounter& dayCounter);
        LocalConstantVol(const Date& referenceDate,
                         const Handle<Quote>& volatility,
                         const DayCounter& dayCounter);
        LocalConstantVol(Integer settlementDays, const Calendar&,
                         Volatility volatility,
                         const DayCounter& dayCounter);
        LocalConstantVol(Integer settlementDays, const Calendar&,
                         const Handle<Quote>& volatility,
                         const DayCounter& dayCounter);
        #endif
        LocalConstantVol(const Date& referenceDate,
                         Volatility volatility,
                         const DayCounter& dayCounter);
        LocalConstantVol(const Date& referenceDate,
                         const Handle<Quote>& volatility,
                         const DayCounter& dayCounter);
        LocalConstantVol(Integer settlementDays, const Calendar&,
                         Volatility volatility,
                         const DayCounter& dayCounter);
        LocalConstantVol(Integer settlementDays, const Calendar&,
                         const Handle<Quote>& volatility,
                         const DayCounter& dayCounter);
        //! \name LocalVolTermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        Date maxDate() const { return Date::maxDate(); }
        Real minStrike() const { return QL_MIN_REAL; }
        Real maxStrike() const { return QL_MAX_REAL; }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        Volatility localVolImpl(Time, Real) const;
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
    };

    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    inline LocalConstantVol::LocalConstantVol(const Date& referenceDate,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(referenceDate), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    }

    inline LocalConstantVol::LocalConstantVol(const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline LocalConstantVol::LocalConstantVol(Integer settlementDays,
                                              const Calendar& calendar,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(settlementDays,calendar), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    }

    inline LocalConstantVol::LocalConstantVol(Integer settlementDays,
                                              const Calendar& calendar,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(settlementDays,calendar), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }
    #endif

    inline LocalConstantVol::LocalConstantVol(const Date& referenceDate,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(referenceDate), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    }

    inline LocalConstantVol::LocalConstantVol(const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline LocalConstantVol::LocalConstantVol(Integer settlementDays,
                                              const Calendar& calendar,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(settlementDays,calendar), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    }

    inline LocalConstantVol::LocalConstantVol(Integer settlementDays,
                                              const Calendar& calendar,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : LocalVolTermStructure(settlementDays,calendar), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline void LocalConstantVol::accept(AcyclicVisitor& v) {
        Visitor<LocalConstantVol>* v1 =
            dynamic_cast<Visitor<LocalConstantVol>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            LocalVolTermStructure::accept(v);
    }

    inline Volatility LocalConstantVol::localVolImpl(Time t, Real) const {
        return volatility_->value();
    }

}


#endif
