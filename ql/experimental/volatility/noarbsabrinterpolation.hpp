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

/*! \file noarbsabrinterpolation.hpp
    \brief no arbitrage sabr interpolation interpolation
           between discrete points
*/

#ifndef quantlib_noarbsabr_interpolation_hpp
#define quantlib_noarbsabr_interpolation_hpp

#include <ql/math/interpolations/xabrinterpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/experimental/volatility/noarbsabrsmilesection.hpp>

#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>

namespace QuantLib {

namespace detail {

// we can directly use the smile section as the wrapper
typedef NoArbSabrSmileSection NoArbSabrWrapper;

struct NoArbSabrSpecs {
    Size dimension() { return 4; }
    void defaultValues(std::vector<Real> &params, const Real &forward,
                       const Real expiryTime) {
        return SABRSpecs().defaultValues(params, forward, expiryTime);
    }
    void guess(Array &values, const Real &forward, const Real expiryTime,
               const std::vector<bool> &parameterAreFixed,
               const std::vector<Real> &r) {
        Size j = 0;
        if (!parameterAreFixed[1])
            values[1] = NoArbSabrModel::Constants::beta_min +
                        (NoArbSabrModel::Constants::beta_max -
                         NoArbSabrModel::Constants::beta_min) *
                            r[j++];
        if (!parameterAreFixed[0]) {
            Real sigmaI = NoArbSabrModel::Constants::sigmaI_min +
                          (NoArbSabrModel::Constants::sigmaI_max -
                           NoArbSabrModel::Constants::sigmaI_min) *
                              r[j++];
            values[0] = sigmaI / std::pow(forward, values[1] - 1.0);
        }
        if (!parameterAreFixed[2])
            values[2] = NoArbSabrModel::Constants::nu_min +
                        (NoArbSabrModel::Constants::nu_max -
                         NoArbSabrModel::Constants::nu_min) *
                            r[j++];
        if (!parameterAreFixed[3])
            values[3] = NoArbSabrModel::Constants::rho_min +
                        (NoArbSabrModel::Constants::rho_max -
                         NoArbSabrModel::Constants::rho_min) *
                            r[j++];
    }
    Array inverse(const Array &y, const Real forward) {
        Array x(4);
        x[1] =
            std::sqrt(-std::log((y[1] - NoArbSabrModel::Constants::beta_min) /
                                (NoArbSabrModel::Constants::beta_max -
                                 NoArbSabrModel::Constants::beta_min)));
        x[0] = std::tan((y[0] * std::pow(forward, y[1] - 1.0) -
                         NoArbSabrModel::Constants::sigmaI_min) /
                            (NoArbSabrModel::Constants::sigmaI_max -
                             NoArbSabrModel::Constants::sigmaI_min) *
                            M_PI -
                        M_PI / 2.0);
        x[2] = std::tan((y[2] - NoArbSabrModel::Constants::nu_min) /
                            (NoArbSabrModel::Constants::nu_max -
                             NoArbSabrModel::Constants::nu_min) *
                            M_PI +
                        M_PI / 2.0);
        x[3] = std::tan((y[3] - NoArbSabrModel::Constants::rho_min) /
                            (NoArbSabrModel::Constants::rho_max -
                             NoArbSabrModel::Constants::rho_min) *
                            M_PI +
                        M_PI / 2.0);
        return x;
    }
    Array direct(const Array &x, const Real forward) {
        Array y(4);
        y[1] = std::fabs(x[1]) < -std::log(NoArbSabrModel::Constants::beta_min)
                   ? NoArbSabrModel::Constants::beta_min +
                         (NoArbSabrModel::Constants::beta_max -
                          NoArbSabrModel::Constants::beta_min) *
                             std::exp(-(x[1] * x[1]))
                   : NoArbSabrModel::Constants::beta_min;
        Real sigmaI = NoArbSabrModel::Constants::sigmaI_min +
                      (NoArbSabrModel::Constants::sigmaI_max -
                       NoArbSabrModel::Constants::sigmaI_min) *
                          (std::atan(x[0]) + M_PI / 2.0) / M_PI;
        y[0] = sigmaI / std::pow(forward, y[1] - 1.0);
        y[2] = NoArbSabrModel::Constants::nu_min +
               (NoArbSabrModel::Constants::nu_max -
                NoArbSabrModel::Constants::nu_min) *
                   (std::atan(x[2]) + M_PI / 2.0) / M_PI;
        y[3] = NoArbSabrModel::Constants::rho_min +
               (NoArbSabrModel::Constants::rho_max -
                NoArbSabrModel::Constants::rho_min) *
                   (std::atan(x[3]) + M_PI / 2.0) / M_PI;
        return y;
    }
    typedef NoArbSabrWrapper type;
    boost::shared_ptr<type> instance(const Time t, const Real &forward,
                                     const std::vector<Real> &params) {
        return boost::make_shared<type>(t, forward, params);
    }
};
}

//! no arbitrage sabr smile interpolation between discrete volatility points.
class NoArbSabrInterpolation : public Interpolation {
  public:
    template <class I1, class I2>
    NoArbSabrInterpolation(
        const I1 &xBegin, // x = strikes
        const I1 &xEnd,
        const I2 &yBegin, // y = volatilities
        Time t,           // option expiry
        const Real &forward, Real alpha, Real beta, Real nu, Real rho,
        bool alphaIsFixed, bool betaIsFixed, bool nuIsFixed, bool rhoIsFixed,
        bool vegaWeighted = true,
        const boost::shared_ptr<EndCriteria> &endCriteria =
            boost::shared_ptr<EndCriteria>(),
        const boost::shared_ptr<OptimizationMethod> &optMethod =
            boost::shared_ptr<OptimizationMethod>(),
        const Real errorAccept = 0.0020, const bool useMaxError = false,
        const Size maxGuesses = 50) {

        impl_ = boost::shared_ptr<Interpolation::Impl>(
            new detail::XABRInterpolationImpl<I1, I2, detail::NoArbSabrSpecs>(
                xBegin, xEnd, yBegin, t, forward,
                boost::assign::list_of(alpha)(beta)(nu)(rho),
                boost::assign::list_of(alphaIsFixed)(betaIsFixed)(nuIsFixed)(
                    rhoIsFixed),
                vegaWeighted, endCriteria, optMethod, errorAccept, useMaxError,
                maxGuesses));
        coeffs_ = boost::dynamic_pointer_cast<
            detail::XABRCoeffHolder<detail::NoArbSabrSpecs> >(impl_);
    }
    Real expiry() const { return coeffs_->t_; }
    Real forward() const { return coeffs_->forward_; }
    Real alpha() const { return coeffs_->params_[0]; }
    Real beta() const { return coeffs_->params_[1]; }
    Real nu() const { return coeffs_->params_[2]; }
    Real rho() const { return coeffs_->params_[3]; }
    Real rmsError() const { return coeffs_->error_; }
    Real maxError() const { return coeffs_->maxError_; }
    const std::vector<Real> &interpolationWeights() const {
        return coeffs_->weights_;
    }
    EndCriteria::Type endCriteria() { return coeffs_->XABREndCriteria_; }

  private:
    boost::shared_ptr<detail::XABRCoeffHolder<detail::NoArbSabrSpecs> > coeffs_;
};

//! no arbtrage sabr interpolation factory and traits
class NoArbSabr {
  public:
    NoArbSabr(Time t, Real forward, Real alpha, Real beta, Real nu, Real rho,
              bool alphaIsFixed, bool betaIsFixed, bool nuIsFixed,
              bool rhoIsFixed, bool vegaWeighted = false,
              const boost::shared_ptr<EndCriteria> endCriteria =
                  boost::shared_ptr<EndCriteria>(),
              const boost::shared_ptr<OptimizationMethod> optMethod =
                  boost::shared_ptr<OptimizationMethod>(),
              const Real errorAccept = 0.0020, const bool useMaxError = false,
              const Size maxGuesses = 50)
        : t_(t), forward_(forward), alpha_(alpha), beta_(beta), nu_(nu),
          rho_(rho), alphaIsFixed_(alphaIsFixed), betaIsFixed_(betaIsFixed),
          nuIsFixed_(nuIsFixed), rhoIsFixed_(rhoIsFixed),
          vegaWeighted_(vegaWeighted), endCriteria_(endCriteria),
          optMethod_(optMethod), errorAccept_(errorAccept),
          useMaxError_(useMaxError), maxGuesses_(maxGuesses) {}
    template <class I1, class I2>
    Interpolation interpolate(const I1 &xBegin, const I1 &xEnd,
                              const I2 &yBegin) const {
        return NoArbSabrInterpolation(
            xBegin, xEnd, yBegin, t_, forward_, alpha_, beta_, nu_, rho_,
            alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_, vegaWeighted_,
            endCriteria_, optMethod_, errorAccept_, useMaxError_, maxGuesses_);
    }
    static const bool global = true;

  private:
    Time t_;
    Real forward_;
    Real alpha_, beta_, nu_, rho_;
    bool alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_;
    bool vegaWeighted_;
    const boost::shared_ptr<EndCriteria> endCriteria_;
    const boost::shared_ptr<OptimizationMethod> optMethod_;
    const Real errorAccept_;
    const bool useMaxError_;
    const Size maxGuesses_;
};
}

#endif
