/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/ShortRateModels/OneFactorModels/vasicek.hpp>
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib {

    Vasicek::Vasicek(Rate r0, Real a, Real b, Real sigma)
    : OneFactorAffineModel(3), r0_(r0),
      a_(arguments_[0]), b_(arguments_[1]), sigma_(arguments_[2]) {
        a_ = ConstantParameter(a, PositiveConstraint());
        b_ = ConstantParameter(b, NoConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
    }

    Real Vasicek::A(Time t, Time T) const {
        Real sigma2 = sigma()*sigma();
        Real bt = B(t, T);
        return std::exp((b() - 0.5*sigma2/(a()*a()))*(bt - (T - t)) -
                        0.25*sigma2*bt*bt/a());
    }

    Real Vasicek::B(Time t, Time T) const {
        return (1.0 - std::exp(-a()*(T - t)))/a();
    }

    Real Vasicek::discountBondOption(Option::Type type,
                                     Real strike, Time maturity,
                                     Time bondMaturity) const {

        Real v;
        if (std::fabs(maturity) < QL_EPSILON) {
            v = 0.0;
        } else {
            v = sigma()*B(maturity, bondMaturity)*
                std::sqrt(0.5*(1.0 - std::exp(-2.0*a()*maturity))/a());
        }
        Real f = discountBond(0.0, bondMaturity, r0_);
        Real k = discountBond(0.0, maturity, r0_)*strike;

        Real w = (type==Option::Call)? 1.0 : -1.0;

        return BlackModel::formula(f, k, v, w);
    }

}
