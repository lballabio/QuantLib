/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

#include <ql/models/equity/roughhestonmodel.hpp>
#include <utility>

namespace QuantLib {

    RoughHestonModel::RoughHestonModel(Handle<YieldTermStructure> riskFreeRate,
                                       Handle<YieldTermStructure> dividendYield,
                                       Handle<Quote> s0,
                                       Real v0,
                                       Real kappa,
                                       Real theta,
                                       Real sigma,
                                       Real rho,
                                       Real hurst)
    : CalibratedModel(6), riskFreeRate_(std::move(riskFreeRate)),
      dividendYield_(std::move(dividendYield)), s0_(std::move(s0)) {
        arguments_[0] = ConstantParameter(theta, PositiveConstraint());
        arguments_[1] = ConstantParameter(kappa, PositiveConstraint());
        arguments_[2] = ConstantParameter(sigma, PositiveConstraint());
        arguments_[3] = ConstantParameter(rho, BoundaryConstraint(-1.0, 1.0));
        arguments_[4] = ConstantParameter(v0, PositiveConstraint());
        arguments_[5] = ConstantParameter(hurst, BoundaryConstraint(0.0, 0.5));

        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(s0_);
    }
}
