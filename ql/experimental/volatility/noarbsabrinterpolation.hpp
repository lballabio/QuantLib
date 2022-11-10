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
    \brief noabr sabr interpolation between discrete points
*/

#ifndef quantlib_noarbsabr_interpolation_hpp
#define quantlib_noarbsabr_interpolation_hpp

#include <ql/experimental/volatility/noarbsabrsmilesection.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <utility>

namespace QuantLib {

namespace detail {

// we can directly use the smile section as the wrapper
typedef NoArbSabrSmileSection NoArbSabrWrapper;

struct NoArbSabrSpecs {
    Size dimension() { return 4; }
    Real eps() { return 0.000001; }
    void defaultValues(std::vector<Real> &params,
                       std::vector<bool> &paramIsFixed, const Real &forward,
                       const Real expiryTime, const std::vector<Real> &addParams) {
        SABRSpecs().defaultValues(params, paramIsFixed, forward, expiryTime, addParams);
        // check if alpha / beta is admissable, otherwise adjust
        // if possible (i.e. not fixed, otherwise an exception will
        // be thrown from the model constructor anyway)
        Real sigmaI = params[0] * std::pow(forward, params[1] - 1.0);
        if (sigmaI < detail::NoArbSabrModel::sigmaI_min) {
            if (!paramIsFixed[0])
                params[0] = detail::NoArbSabrModel::sigmaI_min * (1.0 + eps()) /
                            std::pow(forward, params[1] - 1.0);
            else {
                if (!paramIsFixed[1])
                    params[1] = 1.0 +
                                std::log(detail::NoArbSabrModel::sigmaI_min *
                                         (1.0 + eps()) / params[0]) /
                                    std::log(forward);
            }
        }
        if (sigmaI > detail::NoArbSabrModel::sigmaI_max) {
            if (!paramIsFixed[0])
                params[0] = detail::NoArbSabrModel::sigmaI_max * (1.0 - eps()) /
                            std::pow(forward, params[1] - 1.0);
            else {
                if (!paramIsFixed[1])
                    params[1] = 1.0 +
                                std::log(detail::NoArbSabrModel::sigmaI_max *
                                         (1.0 - eps()) / params[0]) /
                                    std::log(forward);
            }
        }
    }
    void guess(Array &values, const std::vector<bool> &paramIsFixed,
               const Real &forward, const Real expiryTime,
               const std::vector<Real> &r, const std::vector<Real> &) {
        Size j = 0;
        if (!paramIsFixed[1])
            values[1] = detail::NoArbSabrModel::beta_min +
                        (detail::NoArbSabrModel::beta_max -
                         detail::NoArbSabrModel::beta_min) *
                            r[j++];
        if (!paramIsFixed[0]) {
            Real sigmaI = detail::NoArbSabrModel::sigmaI_min +
                          (detail::NoArbSabrModel::sigmaI_max -
                           detail::NoArbSabrModel::sigmaI_min) *
                              r[j++];
            sigmaI *= (1.0 - eps());
            sigmaI += eps() / 2.0;
            values[0] = sigmaI / std::pow(forward, values[1] - 1.0);
        }
        if (!paramIsFixed[2])
            values[2] = detail::NoArbSabrModel::nu_min +
                        (detail::NoArbSabrModel::nu_max -
                         detail::NoArbSabrModel::nu_min) *
                            r[j++];
        if (!paramIsFixed[3])
            values[3] = detail::NoArbSabrModel::rho_min +
                        (detail::NoArbSabrModel::rho_max -
                         detail::NoArbSabrModel::rho_min) *
                            r[j++];
    }
    Array inverse(const Array &y, const std::vector<bool> &paramIsFixed,
                  const std::vector<Real> &params, const Real forward) {
        Array x(4);
        x[1] = std::tan((y[1] - detail::NoArbSabrModel::beta_min) /
                            (detail::NoArbSabrModel::beta_max -
                             detail::NoArbSabrModel::beta_min) *
                            M_PI +
                        M_PI / 2.0);
        x[0] = std::tan((y[0] * std::pow(forward, y[1] - 1.0) -
                         detail::NoArbSabrModel::sigmaI_min) /
                            (detail::NoArbSabrModel::sigmaI_max -
                             detail::NoArbSabrModel::sigmaI_min) *
                            M_PI -
                        M_PI / 2.0);
        x[2] = std::tan((y[2] - detail::NoArbSabrModel::nu_min) /
                            (detail::NoArbSabrModel::nu_max -
                             detail::NoArbSabrModel::nu_min) *
                            M_PI +
                        M_PI / 2.0);
        x[3] = std::tan((y[3] - detail::NoArbSabrModel::rho_min) /
                            (detail::NoArbSabrModel::rho_max -
                             detail::NoArbSabrModel::rho_min) *
                            M_PI +
                        M_PI / 2.0);
        return x;
    }
    Array direct(const Array &x, const std::vector<bool> &paramIsFixed,
                 const std::vector<Real> &params, const Real forward) {
        Array y(4);
        if (paramIsFixed[1])
            y[1] = params[1];
        else
            y[1] = detail::NoArbSabrModel::beta_min +
                   (detail::NoArbSabrModel::beta_max -
                    detail::NoArbSabrModel::beta_min) *
                       (std::atan(x[1]) + M_PI / 2.0) / M_PI;
        // we compute alpha from sigmaI using beta
        // if alpha is fixed we have to check if beta is admissable
        // and adjust if need be
        if (paramIsFixed[0]) {
            y[0] = params[0];
            Real sigmaI = y[0] * std::pow(forward, y[1] - 1.0);
            if (sigmaI < detail::NoArbSabrModel::sigmaI_min) {
                y[1] = (1.0 +
                        std::log(detail::NoArbSabrModel::sigmaI_min *
                                 (1.0 + eps()) / y[0]) /
                            std::log(forward));
            }
            if (sigmaI > detail::NoArbSabrModel::sigmaI_max) {
                y[1] = (1.0 +
                        std::log(detail::NoArbSabrModel::sigmaI_max *
                                 (1.0 - eps()) / y[0]) /
                            std::log(forward));
            }
        } else {
            Real sigmaI = detail::NoArbSabrModel::sigmaI_min +
                          (detail::NoArbSabrModel::sigmaI_max -
                           detail::NoArbSabrModel::sigmaI_min) *
                              (std::atan(x[0]) + M_PI / 2.0) / M_PI;
            y[0] = sigmaI / std::pow(forward, y[1] - 1.0);
        }
        if (paramIsFixed[2])
            y[2] = params[2];
        else
            y[2] = detail::NoArbSabrModel::nu_min +
                   (detail::NoArbSabrModel::nu_max -
                    detail::NoArbSabrModel::nu_min) *
                       (std::atan(x[2]) + M_PI / 2.0) / M_PI;
        if (paramIsFixed[3])
            y[3] = params[3];
        else
            y[3] = detail::NoArbSabrModel::rho_min +
                   (detail::NoArbSabrModel::rho_max -
                    detail::NoArbSabrModel::rho_min) *
                       (std::atan(x[3]) + M_PI / 2.0) / M_PI;
        return y;
    }
    Real weight(const Real strike, const Real forward, const Real stdDev,
                const std::vector<Real> &addParams) {
        return blackFormulaStdDevDerivative(strike, forward, stdDev, 1.0);
    }
    typedef NoArbSabrWrapper type;
    ext::shared_ptr<type> instance(const Time t, const Real &forward,
                                     const std::vector<Real> &params,
                                     const std::vector<Real> &) {
        return ext::make_shared<type>(t, forward, params);
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
        const ext::shared_ptr<EndCriteria> &endCriteria =
            ext::shared_ptr<EndCriteria>(),
        const ext::shared_ptr<OptimizationMethod> &optMethod =
            ext::shared_ptr<OptimizationMethod>(),
        const Real errorAccept = 0.0020, const bool useMaxError = false,
        const Size maxGuesses = 50, const Real shift = 0.0) {

        QL_REQUIRE(shift==0.0,"NoArbSabrInterpolation for non zero shift not implemented");
        impl_ = ext::shared_ptr<Interpolation::Impl>(
            new detail::XABRInterpolationImpl<I1, I2, detail::NoArbSabrSpecs>(
                xBegin, xEnd, yBegin, t, forward,
                {alpha, beta, nu, rho},
                {alphaIsFixed, betaIsFixed, nuIsFixed, rhoIsFixed},
                vegaWeighted, endCriteria, optMethod, errorAccept, useMaxError,
                maxGuesses));
    }
    Real expiry() const { return coeffs().t_; }
    Real forward() const { return coeffs().forward_; }
    Real alpha() const { return coeffs().params_[0]; }
    Real beta() const { return coeffs().params_[1]; }
    Real nu() const { return coeffs().params_[2]; }
    Real rho() const { return coeffs().params_[3]; }
    Real rmsError() const { return coeffs().error_; }
    Real maxError() const { return coeffs().maxError_; }
    const std::vector<Real> &interpolationWeights() const {
        return coeffs().weights_;
    }
    EndCriteria::Type endCriteria() { return coeffs().XABREndCriteria_; }

  private:
    const detail::XABRCoeffHolder<detail::NoArbSabrSpecs>& coeffs() const {
        return *dynamic_cast<detail::XABRCoeffHolder<detail::NoArbSabrSpecs>*>(impl_.get());
    }
};

//! no arbtrage sabr interpolation factory and traits
class NoArbSabr {
  public:
    NoArbSabr(Time t,
              Real forward,
              Real alpha,
              Real beta,
              Real nu,
              Real rho,
              bool alphaIsFixed,
              bool betaIsFixed,
              bool nuIsFixed,
              bool rhoIsFixed,
              bool vegaWeighted = false,
              ext::shared_ptr<EndCriteria> endCriteria = ext::shared_ptr<EndCriteria>(),
              ext::shared_ptr<OptimizationMethod> optMethod = ext::shared_ptr<OptimizationMethod>(),
              const Real errorAccept = 0.0020,
              const bool useMaxError = false,
              const Size maxGuesses = 50)
    : t_(t), forward_(forward), alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
      alphaIsFixed_(alphaIsFixed), betaIsFixed_(betaIsFixed), nuIsFixed_(nuIsFixed),
      rhoIsFixed_(rhoIsFixed), vegaWeighted_(vegaWeighted), endCriteria_(std::move(endCriteria)),
      optMethod_(std::move(optMethod)), errorAccept_(errorAccept), useMaxError_(useMaxError),
      maxGuesses_(maxGuesses) {}
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
    const ext::shared_ptr<EndCriteria> endCriteria_;
    const ext::shared_ptr<OptimizationMethod> optMethod_;
    const Real errorAccept_;
    const bool useMaxError_;
    const Size maxGuesses_;
};
}

#endif
