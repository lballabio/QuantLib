/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/models/shortrate/onefactormodels/vasicek.hpp>
#include <ql/pricingengines/blackformula.hpp>

namespace QuantLib {

    Vasicek::Vasicek(Rate r0, Real a, Real b, Real sigma, Real lambda)
    : OneFactorAffineModel(4), r0_(r0),
      a_(arguments_[0]), b_(arguments_[1]), sigma_(arguments_[2]),
      lambda_(arguments_[3]) {
        a_ = ConstantParameter(a, PositiveConstraint());
        b_ = ConstantParameter(b, NoConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
        lambda_ = ConstantParameter(lambda, NoConstraint());
    }

    Real Vasicek::A(Time t, Time T) const {
        Real _a = a();
        if (_a < std::sqrt(QL_EPSILON)) {
            return 0.0;
        } else {
            Real sigma2 = sigma()*sigma();
            Real bt = B(t, T);
            return std::exp((b() + lambda()*sigma()/_a
                             - 0.5*sigma2/(_a*_a))*(bt - (T - t))
                             - 0.25*sigma2*bt*bt/_a);
        }
    }

    Real Vasicek::B(Time t, Time T) const {
        Real _a = a();
        if (_a < std::sqrt(QL_EPSILON))
            return (T - t);
        else
            return (1.0 - std::exp(-_a*(T - t)))/_a;
    }

    Real Vasicek::discountBondOption(Option::Type type,
                                     Real strike, Time maturity,
                                     Time bondMaturity) const {

        Real v;
        Real _a = a();
        if (std::fabs(maturity) < QL_EPSILON) {
            v = 0.0;
        } else if (_a < std::sqrt(QL_EPSILON)) {
            v = sigma()*B(maturity, bondMaturity)* std::sqrt(maturity);
        } else {
            v = sigma()*B(maturity, bondMaturity)*
                std::sqrt(0.5*(1.0 - std::exp(-2.0*_a*maturity))/_a);
        }
        Real f = discountBond(0.0, bondMaturity, r0_);
        Real k = discountBond(0.0, maturity, r0_)*strike;

        return blackFormula(type, k, f, v);
    }

}

