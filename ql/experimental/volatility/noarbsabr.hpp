/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file noarbsabr.hpp
    \brief No-arbitrage SABR

    Reference: Paul Doust, No-arbitrage SABR,
               The Journal of Computational Finance (3–31)
               Volume 15/Number 3, Spring 2012

    The parameters are bounded as follows (see also below)

    beta [0.01, 0.99]
    expiryTime (0.0, 30.0]
    sigmaI = alpha*forward^(beta-1) [0.05, 1.0]
    nu [0.0001, 0.8]
    rho [-0.99, 0.99]

    As suggested in the paper, d0 is interpolated (linearly)
    in phi space. For beta > 0.9 phi is extrapolated to a
    value corresponding to d0 = tiny_prob = 1E-5 at beta = 1.
    For tau < 0.25 phi is extrapolated flat.
    For rho outside [-0.75, 0.75] phi is extrapolated linearly.

    There are some parameter sets that are admissable, yet do
    not allow for the adjustment procedure as suggested in the
    paper to force the model implied forward to the correct
    value. In this case, no adjustment is done, leading to a
    model implied forward different from the desired one.
    This situation can be identified by comparing forward()
    and numericalForward().
*/

#ifndef quantlib_noarb_sabr
#define quantlib_noarb_sabr

#include <ql/qldefines.hpp>
#include <ql/types.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>

#include <vector>

namespace QuantLib {

namespace detail {
namespace NoArbSabrModel {
// parameter bounds
const Real beta_min = 0.01;
const Real beta_max = 0.99;
const Real expiryTime_max = 30.0;
const Real sigmaI_min = 0.05;
const Real sigmaI_max = 1.00;
const Real nu_min = 0.01;
const Real nu_max = 0.80;
const Real rho_min = -0.99;
const Real rho_max = 0.99;
// cutoff for phi(d0) / tau
// if beta = 0.99, d0 is below 1E-14 for
// bigger values than this
const Real phiByTau_cutoff = 124.587;
// number of mc simulations in tabulated
// absorption probabilities
const Real nsim = 2500000.0;
// small probability used for extrapolation
// of beta towards 1
const Real tiny_prob = 1E-5;
// minimum strike used for normal case integration
const Real strike_min = 1E-6;
// accuracy and max iterations for
// numerical integration
const Real i_accuracy = 1E-7;
const Size i_max_iterations = 10000;
// accuracy when adjusting the model forward
// to match the given forward
const Real forward_accuracy = 1E-6;
// step for searching the model forward
// in newton algorithm
const Real forward_search_step = 0.0010;
// lower bound for density evaluation
const Real density_lower_bound = 1E-50;
// threshold to identify a zero density
const Real density_threshold = 1E-100;
}
}

class NoArbSabrModel {

  public:
    NoArbSabrModel(Real expiryTime, Real forward, Real alpha, Real beta, Real nu, Real rho);

    Real optionPrice(Real strike) const;
    Real digitalOptionPrice(Real strike) const;
    Real density(const Real strike) const {
        return p(strike) * (1 - absProb_) / numericalIntegralOverP_;
    }

    Real forward() const { return externalForward_; }
    Real numericalForward() const { return numericalForward_; }
    Real expiryTime() const { return expiryTime_; }
    Real alpha() const { return alpha_; }
    Real beta() const { return beta_; }
    Real nu() const { return nu_; }
    Real rho() const { return rho_; }

    Real absorptionProbability() const { return absProb_; }

    private:
      Real p(Real f) const;
      Real forwardError(Real forward) const;
      const Real expiryTime_, externalForward_;
      const Real alpha_, beta_, nu_, rho_;
      Real absProb_, fmin_, fmax_;
      mutable Real forward_, numericalIntegralOverP_;
      mutable Real numericalForward_;
      ext::shared_ptr<GaussLobattoIntegral> integrator_;
      class integrand;
      friend class integrand;
      class p_integrand;
      friend class p_integrand;
};

namespace detail {

extern "C" const unsigned long sabrabsprob[1209600];

class D0Interpolator {
  public:
    D0Interpolator(Real forward, Real expiryTime, Real alpha, Real beta, Real nu, Real rho);
    Real operator()() const;

  private:
    Real phi(Real d0) const;
    Real d0(Real phi) const;
    const Real forward_, expiryTime_, alpha_, beta_, nu_, rho_, gamma_;
    Real sigmaI_;
    std::vector<Real> tauG_, sigmaIG_, rhoG_, nuG_, betaG_;
};

} // namespace detail
} // namespace QuantLib

#endif


#ifndef id_828294fde2b48b130d83038ac8b78d28
#define id_828294fde2b48b130d83038ac8b78d28
inline bool test_828294fde2b48b130d83038ac8b78d28(const int* i) {
    return i != nullptr;
}
#endif
