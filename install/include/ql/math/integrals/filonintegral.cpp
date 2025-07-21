/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file filonintegral.cpp
    \brief Filon's formulae for sine and cosine Integrals
*/

#include <ql/errors.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/array.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/filonintegral.hpp>

#include <cmath>

namespace QuantLib {
    FilonIntegral::FilonIntegral(Type type, Real t, Size intervals)
    : Integrator(Null<Real>(), intervals+1),
      type_(type),
      t_(t),
      intervals_(intervals),
      n_        (intervals/2){
        QL_REQUIRE( !(intervals_ & 1), "number of intervals must be even");
    }

    Real FilonIntegral::integrate(const std::function<Real (Real)>& f,
                                  Real a, Real b) const {
        const Real h = (b-a)/(2*n_);
        Array x(2*n_+1, a, h);

        const Real theta = t_*h;
        const Real theta2 = theta*theta;
        const Real theta3 = theta2*theta;

        const Real alpha = 1/theta + std::sin(2*theta)/(2*theta2)
            - 2*squared(std::sin(theta))/theta3;
        const Real beta = 2*( (1+squared(std::cos(theta)))/theta2
            - std::sin(2*theta)/theta3);
        const Real gamma = 4*(std::sin(theta)/theta3 - std::cos(theta)/theta2);

        Array v(x.size());
        std::transform(x.begin(), x.end(), v.begin(), f);

        std::function<Real(Real)> f1, f2;
        switch(type_) {
          case Cosine:
            f1 = [](Real x) -> Real { return std::sin(x); };
            f2 = [](Real x) -> Real { return std::cos(x); };
            break;
          case Sine:
            f1 = [](Real x) -> Real { return std::cos(x); };
            f2 = [](Real x) -> Real { return std::sin(x); };
            break;
          default:
            QL_FAIL("unknown integration type");
        }

        Real c_2n_1 = 0.0;
        Real c_2n = v[0]*f2(t_*a)
            - 0.5*(v[2*n_]*f2(t_*b) + v[0]*f2(t_*a));

        for (Size i=1; i <= n_; ++i) {
            c_2n   += v[2*i]  *f2(t_*x[2*i]);
            c_2n_1 += v[2*i-1]*f2(t_*x[2*i-1]);
        }

        return h*(alpha*(v[2*n_]*f1(t_*x[2*n_]) - v[0]*f1(t_*x[0]))
                  *((type_ == Cosine) ? 1.0 : -1.0)
                 + beta*c_2n + gamma*c_2n_1);
    }
}
