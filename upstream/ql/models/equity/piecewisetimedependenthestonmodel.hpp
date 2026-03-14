/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file piecewisetimedependenthestonmodel.hpp
    \brief piecewise constant time dependent Heston-model
*/

#ifndef quantlib_piecewise_time_dependent_heston_model_hpp
#define quantlib_piecewise_time_dependent_heston_model_hpp

#include <ql/timegrid.hpp>
#include <ql/models/model.hpp>

namespace QuantLib {

    //! Piecewise time dependent Heston model
    /*! References:

        Heston, Steven L., 1993. A Closed-Form Solution for Options
        with Stochastic Volatility with Applications to Bond and
        Currency Options.  The review of Financial Studies, Volume 6,
        Issue 2, 327-343.
        
        A. Elices, Models with time-dependent parameters using 
        transform methods: application to Heston’s model,
        http://arxiv.org/pdf/0708.2020
    */
    class PiecewiseTimeDependentHestonModel : public CalibratedModel {
      public:
        PiecewiseTimeDependentHestonModel(const Handle<YieldTermStructure>& riskFreeRate,
                                          const Handle<YieldTermStructure>& dividendYield,
                                          const Handle<Quote>& s0,
                                          Real v0,
                                          const Parameter& theta,
                                          const Parameter& kappa,
                                          const Parameter& sigma,
                                          const Parameter& rho,
                                          TimeGrid timeGrid);

        // variance mean version level
        Real theta(Time t) const { return arguments_[0](t); }
        // variance mean reversion speed
        Real kappa(Time t) const { return arguments_[1](t); }
        // volatility of the volatility
        Real sigma(Time t) const { return arguments_[2](t); }
        // correlation
        Real rho(Time t)   const { return arguments_[3](t); }
        // spot variance
        Real v0()          const { return arguments_[4](0.0); }
        // spot
        Real s0()          const { return s0_->value(); }

        
        const TimeGrid& timeGrid() const;
        const Handle<YieldTermStructure>& dividendYield() const;
        const Handle<YieldTermStructure>& riskFreeRate() const;
        
      protected:
        const Handle<Quote> s0_;
        const Handle<YieldTermStructure> riskFreeRate_;
        const Handle<YieldTermStructure> dividendYield_;
        const TimeGrid timeGrid_;
    };
}


#endif

