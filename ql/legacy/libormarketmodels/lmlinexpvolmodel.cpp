/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/lmlinexpvolmodel.hpp>

namespace QuantLib {

    LmLinearExponentialVolatilityModel::LmLinearExponentialVolatilityModel(
                                         const std::vector<Time>& fixingTimes,
                                         Real a, Real b, Real c, Real d)
    : LmVolatilityModel(fixingTimes.size(), 4),
      fixingTimes_(fixingTimes) {
        arguments_[0] = ConstantParameter(a, PositiveConstraint());
        arguments_[1] = ConstantParameter(b, PositiveConstraint());
        arguments_[2] = ConstantParameter(c, PositiveConstraint());
        arguments_[3] = ConstantParameter(d, PositiveConstraint());
    }


    Disposable<Array> LmLinearExponentialVolatilityModel::volatility(
                                                 Time t, const Array&) const {
        const Real a = arguments_[0](0.0);
        const Real b = arguments_[1](0.0);
        const Real c = arguments_[2](0.0);
        const Real d = arguments_[3](0.0);

        Array tmp(size_, 0.0);

        for (Size i=0; i<size_; ++i) {
            const Time T = fixingTimes_[i];
            if (T>t) {
                tmp[i] = (a*(T-t)+d)*std::exp(-b*(T-t)) + c;
            }
        }

        return tmp;
    }

    Volatility LmLinearExponentialVolatilityModel::volatility(
                                         Size i, Time t, const Array&) const {
        const Real a = arguments_[0](0.0);
        const Real b = arguments_[1](0.0);
        const Real c = arguments_[2](0.0);
        const Real d = arguments_[3](0.0);

        const Time T = fixingTimes_[i];

        return (T>t) ? (a*(T-t)+d)*std::exp(-b*(T-t)) + c : 0.0;
    }

    Real LmLinearExponentialVolatilityModel::integratedVariance(
                                 Size i, Size j, Time u, const Array&) const {
        const Real a = arguments_[0](0.0);
        const Real b = arguments_[1](0.0);
        const Real c = arguments_[2](0.0);
        const Real d = arguments_[3](0.0);

        const Time T = fixingTimes_[i];
        const Time S = fixingTimes_[j];

        const Real k1=std::exp(b*u);
        const Real k2=std::exp(b*S);
        const Real k3=std::exp(b*T);

        return (a*a*(-1 - 2*b*b*S*T - b*(S + T)
                     + k1*k1*(1 + b*(S + T - 2*u) + 2*b*b*(S - u)*(T - u)))
                + 2*b*b*(2*c*d*(k2 + k3)*(k1 - 1)
                         +d*d*(k1*k1 - 1)+2*b*c*c*k2*k3*u)
                + 2*a*b*(d*(-1 - b*(S + T) + k1*k1*(1 + b*(S + T - 2*u)))
                         -2*c*(k3*(1 + b*S) + k2*(1 + b*T)
                               - k1*k3*(1 + b*(S - u))
                               - k1*k2*(1 + b*(T - u)))
                         )
                ) / (4*b*b*b*k2*k3);
    }

    void LmLinearExponentialVolatilityModel::generateArguments() {}

}

