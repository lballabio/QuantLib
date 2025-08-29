/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sabrvoltermstructure.hpp
    \brief implied vol surface backed by a SABR model
*/

#ifndef quantlib_sabr_vol_termstructure_hpp
#define quantlib_sabr_vol_termstructure_hpp


#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {
    class SABRVolTermStructure : public BlackVolatilityTermStructure {
      public:
        SABRVolTermStructure(Real alpha,
                             Real beta,
                             Real gamma,
                             Real rho,
                             Real s0,
                             Real r,
                             const Date& referenceDate,
                             const DayCounter& dc)
        : BlackVolatilityTermStructure(referenceDate, NullCalendar(), Following, dc), alpha_(alpha),
          beta_(beta), gamma_(gamma), rho_(rho), s0_(s0), r_(r) {}

        Date maxDate() const override { return Date::maxDate(); }
        Rate minStrike() const override { return 0.0; }
        Rate maxStrike() const override { return QL_MAX_REAL; }

      protected:
        Volatility blackVolImpl(Time t, Real strike) const override {
            const Real fwd = s0_*std::exp(r_*t);
            return sabrVolatility(strike, fwd, t, alpha_, beta_, gamma_, rho_);
        }

      private:
        Real alpha_, beta_, gamma_, rho_, s0_, r_;
    };
}

#endif
