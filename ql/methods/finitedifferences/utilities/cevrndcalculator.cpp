/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file cevrndcalculator.cpp */

#include <ql/errors.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

#include <ql/methods/finitedifferences/utilities/cevrndcalculator.hpp>

#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>

namespace QuantLib {

    CEVRNDCalculator::CEVRNDCalculator(Real f0, Real alpha, Real beta)
    : f0_(f0),
      alpha_(alpha),
      beta_(beta),
      delta_((1.0-2.0*beta)/(1.0-beta)),
      x0_(X(f0)) {
        QL_REQUIRE(beta != 1.0, "beta can not be one");
    }

    Real CEVRNDCalculator::massAtZero(Time t) const {
        if (delta_ < 2.0)
            return 1.0-boost::math::gamma_p(-0.5*delta_+1.0,x0_/(2.0*t));
        else
            return 0.0;
    }

    Real CEVRNDCalculator::X(Real f) const {
        return std::pow(f, 2.0*(1.0-beta_))/square<Real>()(alpha_*(1.0-beta_));
    }

    Real CEVRNDCalculator::invX(Real x) const {
        return std::pow(x*square<Real>()(alpha_*(1.0-beta_)),
                        1.0/(2.0*(1.0-beta_)));
    }

    Real CEVRNDCalculator::pdf(Real f, Time t) const {
        const Real y = X(f);

        if (delta_ < 2.0) {
            return boost::math::pdf(
                boost::math::non_central_chi_squared_distribution<Real>(
                    4.0-delta_, y/t), x0_/t)/t * 2.0*(1.0-beta_)*y/f;
        }
        else {
            return boost::math::pdf(
                boost::math::non_central_chi_squared_distribution<Real>(
                    delta_, x0_/t), y/t)/t * 2.0*(beta_-1.0)*y/f;
        }
    }

    Real CEVRNDCalculator::cdf(Real f, Time t) const {
        const Real y = X(f);

        if (delta_ < 2.0)
            return 1.0 - boost::math::cdf(
                boost::math::non_central_chi_squared_distribution<Real>(
                    2.0-delta_, y/t), x0_/t);
        else
            return 1.0 - boost::math::cdf(
                boost::math::non_central_chi_squared_distribution<Real>(
                    delta_, x0_/t), y/t);
    }

    Real CEVRNDCalculator::sankaranApprox(Real c, Time t, Real x) const {
        const Real a = x0_/t;
        const Real b = 2.0 - delta_;

        c = std::max(c, -0.45*b);

        const Real h = 1 - 2*(b+c)*(b+3*c)/(3*square<Real>()(b+2*c));
        const Real p = (b+2*c)/square<Real>()(b+c);
        const Real m = (h-1)*(1-3*h);

        const Real u = (std::pow(a/(b+c), h) - (1 + h*p*(h-1-0.5*(2-h)*m*p)))/
                (h*std::sqrt(2*p)*(1+0.5*m*p));

        return u - x;
    }

    Real CEVRNDCalculator::invcdf(Real q, Time t) const {
        if (delta_ < 2.0) {
            if (f0_ < QL_EPSILON || q < massAtZero(t))
                return 0.0;

            const Real x = InverseCumulativeNormal()(1-q);

            auto cdfApprox = [&](Real _c){ return sankaranApprox(_c, t, x); };

            const Real y0 = X(f0_)/t;

            try {
                Brent brent;
                brent.setMaxEvaluations(20);
                const Real guess =
                    invX(brent.solve(cdfApprox, 1e-8, y0, 0.02*y0) * t);

                return InvCDFHelper(this, guess, 1e-8, 100).inverseCDF(q, t);
            }
            catch (...) {
                return InvCDFHelper(this, f0_, 1e-8, 100).inverseCDF(q, t);
            }
        }
        else {
            const Real x = t * boost::math::quantile(
                boost::math::non_central_chi_squared_distribution<Real>(
                    delta_, x0_/t), 1-q);
            return invX(x);
        }
    }
}
