/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano

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

/*! \file localconstantvol.hpp
    \brief Local constant volatility, no time dependence, no asset dependence
*/

#ifndef quantlib_localconstantvol_hpp
#define quantlib_localconstantvol_hpp

#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <utility>

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
        LocalConstantVol(const Date& referenceDate, Volatility volatility, DayCounter dayCounter);
        LocalConstantVol(const Date& referenceDate,
                         Handle<Quote> volatility,
                         DayCounter dayCounter);
        LocalConstantVol(Natural settlementDays,
                         const Calendar&,
                         Volatility volatility,
                         DayCounter dayCounter);
        LocalConstantVol(Natural settlementDays,
                         const Calendar&,
                         Handle<Quote> volatility,
                         DayCounter dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const override { return dayCounter_; }
        Date maxDate() const override { return Date::maxDate(); }
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override { return QL_MIN_REAL; }
        Real maxStrike() const override { return QL_MAX_REAL; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        Volatility localVolImpl(Time, Real) const override;
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
    };

    // inline definitions

    inline LocalConstantVol::LocalConstantVol(const Date& referenceDate,
                                              Volatility volatility,
                                              DayCounter dayCounter)
    : LocalVolTermStructure(referenceDate),
      volatility_(ext::shared_ptr<Quote>(new SimpleQuote(volatility))),
      dayCounter_(std::move(dayCounter)) {}

    inline LocalConstantVol::LocalConstantVol(const Date& referenceDate,
                                              Handle<Quote> volatility,
                                              DayCounter dayCounter)
    : LocalVolTermStructure(referenceDate), volatility_(std::move(volatility)),
      dayCounter_(std::move(dayCounter)) {
        registerWith(volatility_);
    }

    inline LocalConstantVol::LocalConstantVol(Natural settlementDays,
                                              const Calendar& calendar,
                                              Volatility volatility,
                                              DayCounter dayCounter)
    : LocalVolTermStructure(settlementDays, calendar),
      volatility_(ext::shared_ptr<Quote>(new SimpleQuote(volatility))),
      dayCounter_(std::move(dayCounter)) {}

    inline LocalConstantVol::LocalConstantVol(Natural settlementDays,
                                              const Calendar& calendar,
                                              Handle<Quote> volatility,
                                              DayCounter dayCounter)
    : LocalVolTermStructure(settlementDays, calendar), volatility_(std::move(volatility)),
      dayCounter_(std::move(dayCounter)) {
        registerWith(volatility_);
    }

    inline void LocalConstantVol::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<LocalConstantVol>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            LocalVolTermStructure::accept(v);
    }

    inline Volatility LocalConstantVol::localVolImpl(Time, Real) const {
        return volatility_->value();
    }

}


#endif


#ifndef id_23b6c55474176496751b16a4617df479
#define id_23b6c55474176496751b16a4617df479
inline bool test_23b6c55474176496751b16a4617df479(const int* i) {
    return i != nullptr;
}
#endif
