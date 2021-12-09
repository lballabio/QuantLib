/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file bsmrndcalculator.hpp
    \brief risk neutral terminal density calculator for the
           Black-Scholes-Merton model with constant volatility
*/

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/utilities/bsmrndcalculator.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    BSMRNDCalculator::BSMRNDCalculator(ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {}

    std::pair<Real, Volatility>
    BSMRNDCalculator::distributionParams(Real x, Time t) const {
        const Volatility stdDev =
            process_->blackVolatility()->blackVol(t, std::exp(x))*std::sqrt(t);
        const Real mean = std::log(process_->x0()) - 0.5*stdDev*stdDev
            + std::log(  process_->dividendYield()->discount(t)
                       / process_->riskFreeRate()->discount(t));

        return std::make_pair(mean, stdDev);
    }

    Real BSMRNDCalculator::pdf(Real x, Time t) const {
        std::pair<Real, Volatility> p = distributionParams(x, t);
        return NormalDistribution(p.first, p.second)(x);
    }

    Real BSMRNDCalculator::cdf(Real x, Time t) const {
        std::pair<Real, Volatility> p = distributionParams(x, t);
        return CumulativeNormalDistribution(p.first, p.second)(x);
    }

    Real BSMRNDCalculator::invcdf(Real x, Time t) const {
        std::pair<Real, Volatility> p = distributionParams(x, t);
        return InvCumulativeNormalDistribution(p.first, p.second)(x);
    }
}
