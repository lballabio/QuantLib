/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file piecewiseintegral.hpp
    \brief Integral of a piecewise well behaved function using
           a custom integrator for the pieces. It can be forced
           that the function is integrated only over intervals
           strictly not containing the critical points
*/

#ifndef quantlib_piecewise_integral_hpp
#define quantlib_piecewise_integral_hpp

#include <ql/math/integrals/integral.hpp>
#include <ql/math/comparison.hpp>
#include <ql/shared_ptr.hpp>
#include <algorithm>
#include <vector>

namespace QuantLib {

class PiecewiseIntegral : public Integrator {
  public:
    PiecewiseIntegral(ext::shared_ptr<Integrator> integrator,
                      std::vector<Real> criticalPoints,
                      bool avoidCriticalPoints = true);

  protected:
    Real integrate(const ext::function<Real(Real)>& f, Real a, Real b) const override;

  private:
    Real integrate_h(const ext::function<Real(Real)> &f, Real a,
                     Real b) const;
    const ext::shared_ptr<Integrator> integrator_;
    std::vector<Real> criticalPoints_;
    const Real eps_;
};

// inline

inline Real PiecewiseIntegral::integrate_h(const ext::function<Real(Real)> &f,
                                           Real a, Real b) const {

    if (!close_enough(a, b))
        return (*integrator_)(f, a, b);
    else
        return 0.0;
}

inline Real PiecewiseIntegral::integrate(const ext::function<Real(Real)> &f,
                                         Real a, Real b) const {

    auto a0 = std::lower_bound(criticalPoints_.begin(), criticalPoints_.end(), a);

    auto b0 = std::lower_bound(criticalPoints_.begin(), criticalPoints_.end(), b);

    if (a0 == criticalPoints_.end()) {
        Real tmp = 1.0;
        if (!criticalPoints_.empty()) {
            if (close_enough(a, criticalPoints_.back())) {
                tmp = eps_;
            }
        }
        return integrate_h(f, a * tmp, b);
    }

    Real res = 0.0;

    if (!close_enough(a, *a0)) {
        res += integrate_h(f, a, std::min(*a0 / eps_, b));
    }

    if (b0 == criticalPoints_.end()) {
        --b0;
        if (!close_enough(*b0, b)) {
            res += integrate_h(f, (*b0) * eps_, b);
        }
    }

    for (auto x = a0; x < b0; ++x) {
        res += integrate_h(f, (*x) * eps_, std::min(*(x + 1) / eps_, b));
    }

    return res;
}

} // namespace QuantLib

#endif
