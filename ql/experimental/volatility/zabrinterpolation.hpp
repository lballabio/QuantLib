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

/*! \file zabrinterpolation.hpp
    \brief ZABR interpolation interpolation between discrete points
*/

#ifndef quantlib_zabr_interpolation_hpp
#define quantlib_zabr_interpolation_hpp

#include <ql/experimental/volatility/zabrsmilesection.hpp>
#include <ql/math/interpolations/xabrinterpolation.hpp>
#include <utility>

namespace QuantLib {

namespace detail {

template <typename Evaluation> struct ZabrSpecs {
    Size dimension() { return 5; }
    Real eps() { return 0.000001; }
    void defaultValues(std::vector<Real> &params,
                       std::vector<bool> &paramIsFixed, const Real &forward,
                       const Real expiryTime, const std::vector<Real>& addParams) {
        if (params[1] == Null<Real>())
            params[1] = 0.5;
        if (params[0] == Null<Real>())
            // adapt alpha to beta level
            params[0] =
                0.2 *
                (params[1] < 0.9999 ? std::pow(forward, 1.0 - params[1]) : 1.0);
        if (params[2] == Null<Real>())
            params[2] = std::sqrt(0.4);
        if (params[3] == Null<Real>())
            params[3] = 0.0;
        if (params[4] == Null<Real>())
            params[4] = 1.0;
    }
    void guess(Array &values, const std::vector<bool> &paramIsFixed,
               const Real &forward, const Real expiryTime,
               const std::vector<Real> &r, const std::vector<Real>& addParams) {
        Size j = 0;
        if (!paramIsFixed[1])
            values[1] = (1.0 - 2E-6) * r[j++] + 1E-6;
        if (!paramIsFixed[0]) {
            values[0] = (1.0 - 2E-6) * r[j++] + 1E-6; // lognormal vol guess
            // adapt this to beta level
            if (values[1] < 0.999)
                values[0] *= std::pow(forward, 1.0 - values[1]);
        }
        if (!paramIsFixed[2])
            values[2] = 1.5 * r[j++] + 1E-6;
        if (!paramIsFixed[3])
            values[3] = (2.0 * r[j++] - 1.0) * (1.0 - 1E-6);
        if (!paramIsFixed[4])
            values[4] = r[j++] * 2.0;
    }
    Real eps1() { return .0000001; }
    Real eps2() { return .9999; }
    Real dilationFactor() { return 0.001; }
    Array inverse(const Array &y, const std::vector<bool> &,
                  const std::vector<Real> &, const Real) {
        Array x(5);
        x[0] = y[0] < 25.0 + eps1() ? std::sqrt(y[0] - eps1())
                                    : (y[0] - eps1() + 25.0) / 10.0;
        x[1] = std::sqrt(-std::log(y[1]));
        x[2] = std::tan(M_PI*(y[4]/5.0-0.5));
        x[3] = std::asin(y[3] / eps2());
        x[4] = std::tan(M_PI*(y[4]/1.9-0.5));
        return x;
    }
    Array direct(const Array &x, const std::vector<bool> &,
                 const std::vector<Real> &, const Real) {
        Array y(5);
        y[0] = std::fabs(x[0]) < 5.0 ? x[0] * x[0] + eps1()
                                     : (10.0 * std::fabs(x[0]) - 25.0) + eps1();
        y[1] = std::fabs(x[1]) < std::sqrt(-std::log(eps1()))
                   ? std::exp(-(x[1] * x[1]))
                   : eps1();
        // limit nu to 5.00
        y[2] = (std::atan(x[2])/M_PI + 0.5) * 5.0;
        y[3] = std::fabs(x[3]) < 2.5 * M_PI
                   ? eps2() * std::sin(x[3])
                   : eps2() * (x[3] > 0.0 ? 1.0 : (-1.0));
        // limit gamma to 1.9
        y[4] = (std::atan(x[4])/M_PI + 0.5) * 1.9;
        return y;
    }
    Real weight(const Real strike, const Real forward, const Real stdDev,
                const std::vector<Real> &addParams) {
        return blackFormulaStdDevDerivative(strike, forward, stdDev, 1.0);
    }
    typedef ZabrSmileSection<Evaluation> type;
    ext::shared_ptr<type> instance(const Time t, const Real &forward,
                                     const std::vector<Real> &params,
                                     const std::vector<Real> &addParams) {
        return ext::make_shared<type>(t, forward, params);
    }
};
} // end namespace detail


//! zabr smile interpolation between discrete volatility points.
template <class Evaluation> class ZabrInterpolation : public Interpolation {
  public:
    template <class I1, class I2>
    ZabrInterpolation(
        const I1 &xBegin, // x = strikes
        const I1 &xEnd,
        const I2 &yBegin, // y = volatilities
        Time t,           // option expiry
        const Real &forward, Real alpha, Real beta, Real nu, Real rho,
        Real gamma, bool alphaIsFixed, bool betaIsFixed, bool nuIsFixed,
        bool rhoIsFixed, bool gammaIsFixed, bool vegaWeighted = true,
        const ext::shared_ptr<EndCriteria> &endCriteria =
            ext::shared_ptr<EndCriteria>(),
        const ext::shared_ptr<OptimizationMethod> &optMethod =
            ext::shared_ptr<OptimizationMethod>(),
        const Real errorAccept = 0.0020, const bool useMaxError = false,
        const Size maxGuesses = 50) {
            impl_ = ext::shared_ptr<
                Interpolation::Impl>(new detail::XABRInterpolationImpl<
                I1, I2,
                detail::ZabrSpecs<Evaluation> >(
                xBegin, xEnd, yBegin, t, forward,
                {alpha, beta, nu, rho, gamma},
                {alphaIsFixed, betaIsFixed, nuIsFixed, rhoIsFixed, gammaIsFixed},
                vegaWeighted, endCriteria, optMethod, errorAccept, useMaxError,
                maxGuesses));
            coeffs_ = ext::dynamic_pointer_cast<detail::XABRCoeffHolder<
                detail::ZabrSpecs<Evaluation> > >(impl_);
    }
    Real expiry() const { return coeffs_->t_; }
    Real forward() const { return coeffs_->forward_; }
    Real alpha() const { return coeffs_->params_[0]; }
    Real beta() const { return coeffs_->params_[1]; }
    Real nu() const { return coeffs_->params_[2]; }
    Real rho() const { return coeffs_->params_[3]; }
    Real gamma() const { return coeffs_->params_[4]; }
    Real rmsError() const { return coeffs_->error_; }
    Real maxError() const { return coeffs_->maxError_; }
    const std::vector<Real> &interpolationWeights() const {
        return coeffs_->weights_;
    }
    EndCriteria::Type endCriteria() { return coeffs_->XABREndCriteria_; }

  private:
    ext::shared_ptr<detail::XABRCoeffHolder<detail::ZabrSpecs<Evaluation> > > coeffs_;
};

//! no arbtrage sabr interpolation factory and traits
template<class Evaluation> class Zabr {
  public:
    Zabr(Time t,
         Real forward,
         Real alpha,
         Real beta,
         Real nu,
         Real rho,
         Real gamma,
         bool alphaIsFixed,
         bool betaIsFixed,
         bool nuIsFixed,
         bool rhoIsFixed,
         bool gammaIsFixed,
         bool vegaWeighted = false,
         ext::shared_ptr<EndCriteria> endCriteria = ext::shared_ptr<EndCriteria>(),
         ext::shared_ptr<OptimizationMethod> optMethod = ext::shared_ptr<OptimizationMethod>(),
         const Real errorAccept = 0.0020,
         const bool useMaxError = false,
         const Size maxGuesses = 50)
    : t_(t), forward_(forward), alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
      alphaIsFixed_(alphaIsFixed), betaIsFixed_(betaIsFixed), nuIsFixed_(nuIsFixed),
      rhoIsFixed_(rhoIsFixed), gammaIsFixed_(gammaIsFixed), vegaWeighted_(vegaWeighted),
      endCriteria_(std::move(endCriteria)), optMethod_(std::move(optMethod)),
      errorAccept_(errorAccept), useMaxError_(useMaxError), maxGuesses_(maxGuesses) {}
    template <class I1, class I2>
    Interpolation interpolate(const I1 &xBegin, const I1 &xEnd,
                              const I2 &yBegin) const {
        return ZabrInterpolation<Evaluation>(
            xBegin, xEnd, yBegin, t_, forward_, alpha_, beta_, nu_, rho_,
            gamma_, alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_,
            gammaIsFixed_, vegaWeighted_, endCriteria_, optMethod_,
            errorAccept_, useMaxError_, maxGuesses_);
    }
    static const bool global = true;

  private:
    Time t_;
    Real forward_;
    Real alpha_, beta_, nu_, rho_, gamma_;
    bool alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_, gammaIsFixed_;
    bool vegaWeighted_;
    const ext::shared_ptr<EndCriteria> endCriteria_;
    const ext::shared_ptr<OptimizationMethod> optMethod_;
    const Real errorAccept_;
    const bool useMaxError_;
    const Size maxGuesses_;
};
}

#endif


#ifndef id_d7b37573988115bb4128fe468aa8f533
#define id_d7b37573988115bb4128fe468aa8f533
inline bool test_d7b37573988115bb4128fe468aa8f533(int* i) { return i != 0; }
#endif
