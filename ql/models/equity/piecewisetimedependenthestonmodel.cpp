/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

#include <ql/quotes/simplequote.hpp>
#include <ql/models/equity/piecewisetimedependenthestonmodel.hpp>

namespace QuantLib {

    PiecewiseTimeDependentHestonModel::PiecewiseTimeDependentHestonModel(
        const Handle<YieldTermStructure>& riskFreeRate,
        const Handle<YieldTermStructure>& dividendYield,
        const Handle<Quote>& s0,
        Real v0,
        const Parameter& theta,
        const Parameter& kappa,
        const Parameter& sigma,
        const Parameter& rho,
        const TimeGrid& timeGrid)
    : CalibratedModel(5), 
      s0_           (s0),
      riskFreeRate_ (riskFreeRate),
      dividendYield_(dividendYield),
      timeGrid_     (timeGrid) {
            
        arguments_[0] = theta;
        arguments_[1] = kappa;
        arguments_[2] = sigma;
        arguments_[3] = rho;
        arguments_[4] = ConstantParameter(v0, PositiveConstraint());

        registerWith(s0);
        registerWith(riskFreeRate);
        registerWith(dividendYield);
    }

    const TimeGrid& PiecewiseTimeDependentHestonModel::timeGrid() const {
        return timeGrid_;
    }
    
    const Handle<YieldTermStructure>& 
    PiecewiseTimeDependentHestonModel::dividendYield() const {
        return dividendYield_;
    }

    const Handle<YieldTermStructure>& 
    PiecewiseTimeDependentHestonModel::riskFreeRate() const {
        return riskFreeRate_;
    }
}

