/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

/*! \file blackconstantvol.hpp
    \brief Black constant volatility, no time dependence, no strike dependence
*/

#ifndef quantlib_blackconstantvol_hpp
#define quantlib_blackconstantvol_hpp

#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <utility>

namespace QuantLib {

    //! Constant Black volatility, no time-strike dependence
    /*! This class implements the BlackVolatilityTermStructure
        interface for a constant Black volatility (no time/strike
        dependence).
    */
    class BlackConstantVol : public BlackVolatilityTermStructure {
      public:
        BlackConstantVol(const Date& referenceDate,
                         const Calendar&,
                         Volatility volatility,
                         const DayCounter& dayCounter);
        BlackConstantVol(const Date& referenceDate,
                         const Calendar&,
                         Handle<Quote> volatility,
                         const DayCounter& dayCounter);
        BlackConstantVol(Natural settlementDays,
                         const Calendar&,
                         Volatility volatility,
                         const DayCounter& dayCounter);
        BlackConstantVol(Natural settlementDays,
                         const Calendar&,
                         Handle<Quote> volatility,
                         const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Volatility blackVolImpl(Time t, Real) const override;

      private:
        Handle<Quote> volatility_;
    };


    // inline definitions

    inline BlackConstantVol::BlackConstantVol(const Date& referenceDate,
                                              const Calendar& cal,
                                              Volatility volatility,
                                              const DayCounter& dc)
    : BlackVolatilityTermStructure(referenceDate, cal, Following, dc),
      volatility_(ext::shared_ptr<Quote>(new SimpleQuote(volatility))) {}

    inline BlackConstantVol::BlackConstantVol(const Date& referenceDate,
                                              const Calendar& cal,
                                              Handle<Quote> volatility,
                                              const DayCounter& dc)
    : BlackVolatilityTermStructure(referenceDate, cal, Following, dc),
      volatility_(std::move(volatility)) {
        registerWith(volatility_);
    }

    inline BlackConstantVol::BlackConstantVol(Natural settlementDays,
                                              const Calendar& cal,
                                              Volatility volatility,
                                              const DayCounter& dc)
    : BlackVolatilityTermStructure(settlementDays, cal, Following, dc),
      volatility_(ext::shared_ptr<Quote>(new SimpleQuote(volatility))) {}

    inline BlackConstantVol::BlackConstantVol(Natural settlementDays,
                                              const Calendar& cal,
                                              Handle<Quote> volatility,
                                              const DayCounter& dc)
    : BlackVolatilityTermStructure(settlementDays, cal, Following, dc),
      volatility_(std::move(volatility)) {
        registerWith(volatility_);
    }

    inline Date BlackConstantVol::maxDate() const {
        return Date::maxDate();
    }

    inline Real BlackConstantVol::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real BlackConstantVol::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline void BlackConstantVol::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackConstantVol>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVolatilityTermStructure::accept(v);
    }

    inline Volatility BlackConstantVol::blackVolImpl(Time, Real) const {
        return volatility_->value();
    }

}


#endif


#ifndef id_850b44fbfd354a8fb59463ddb6e29886
#define id_850b44fbfd354a8fb59463ddb6e29886
inline bool test_850b44fbfd354a8fb59463ddb6e29886(const int* i) {
    return i != nullptr;
}
#endif
