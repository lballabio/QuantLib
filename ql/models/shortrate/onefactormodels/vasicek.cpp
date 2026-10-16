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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/shortrate/onefactormodels/vasicek.hpp>
#include <ql/pricingengines/blackformula.hpp>

namespace QuantLib {

    namespace {

        // (1 - exp(-x))/x, accurate for small x as well
        Real oneMinusExpOverX(Real x) {
            return x == 0.0 ? 1.0 : -std::expm1(-x)/x;
        }

        // The two functions
        //   phi(x) = (exp(-x) - 1 + x)/x^2
        //   h(x)   = (4 exp(-x) - exp(-2x) - 3 + 2x)/x^3
        // appear in the exponent of A. Their closed forms cancel
        // catastrophically for small x, so we use the Taylor series
        //   phi(x) = sum_k (k+3) p_k
        //   h(x)   = sum_k (2^(k+3) - 4) p_k
        // with p_k = (-x)^k/(k+3)!, which converges quickly and
        // without significant cancellation for |x| < 1.
        void phiAndH(Real x, Real& phi, Real& h) {
            if (std::fabs(x) < 1.0) {
                Real p = 1.0/6.0;
                Real twoPower = 8.0;
                phi = h = 0.0;
                for (Size k=0; k<40; ++k) {
                    const Real hTerm = (twoPower - 4.0)*p;
                    phi += (k + 3.0)*p;
                    h += hTerm;
                    if (std::fabs(hTerm) <= QL_EPSILON*std::fabs(h))
                        break;
                    p *= -x/(k + 4.0);
                    twoPower *= 2.0;
                }
            } else {
                const Real e1 = std::expm1(-x);
                const Real e2 = std::expm1(-2.0*x);
                phi = (e1 + x)/(x*x);
                h = (4.0*e1 - e2 + 2.0*x)/(x*x*x);
            }
        }

    }

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
        // A(t,T) = exp((b + lambda*sigma/a - sigma^2/(2a^2))*(B - tau)
        //               - sigma^2*B^2/(4a))
        // rewritten as exp(-(a*b + lambda*sigma)*tau^2*phi(a*tau)
        //                  + sigma^2*tau^3*h(a*tau)/4),
        // which is free of cancellation and has the correct limit
        // for a -> 0.
        const Real tau = T - t;
        const Real sigma2 = sigma()*sigma();
        Real phi, h;
        phiAndH(a()*tau, phi, h);
        return std::exp(-(a()*b() + lambda()*sigma())*tau*tau*phi +
                        0.25*sigma2*tau*tau*tau*h);
    }

    Real Vasicek::B(Time t, Time T) const {
        const Time tau = T - t;
        return tau*oneMinusExpOverX(a()*tau);
    }

    Real Vasicek::discountBondOption(Option::Type type,
                                     Real strike, Time maturity,
                                     Time bondMaturity) const {

        Real v;
        if (std::fabs(maturity) < QL_EPSILON) {
            v = 0.0;
        } else {
            // 0.5*(1 - exp(-2*a*maturity))/a
            const Real variance = maturity*oneMinusExpOverX(2.0*a()*maturity);
            v = sigma()*B(maturity, bondMaturity)*std::sqrt(variance);
        }
        Real f = discountBond(0.0, bondMaturity, r0_);
        Real k = discountBond(0.0, maturity, r0_)*strike;

        return blackFormula(type, k, f, v);
    }

}
