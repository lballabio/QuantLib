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

#include <ql/experimental/volatility/noarbsabr.hpp>

#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/modifiedbessel.hpp>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/functional/hash.hpp>

namespace QuantLib {

class NoArbSabrModel::integrand {
    const NoArbSabrModel* model;
    Real strike;
  public:
    integrand(const NoArbSabrModel* model, Real strike)
    : model(model), strike(strike) {}
    Real operator()(Real f) const {
        return std::max(f - strike, 0.0) * model->p(f);
    }
};

class NoArbSabrModel::p_integrand {
    const NoArbSabrModel* model;
  public:
    explicit p_integrand(const NoArbSabrModel* model)
    : model(model) {}
    Real operator()(Real f) const {
        return model->p(f);
    }
};

NoArbSabrModel::NoArbSabrModel(const Real expiryTime, const Real forward,
                               const Real alpha, const Real beta, const Real nu,
                               const Real rho)
    : expiryTime_(expiryTime), externalForward_(forward), alpha_(alpha),
      beta_(beta), nu_(nu), rho_(rho), forward_(forward),
      numericalForward_(forward) {

    QL_REQUIRE(expiryTime > 0.0 && expiryTime <= detail::NoArbSabrModel::expiryTime_max,
               "expiryTime (" << expiryTime << ") out of bounds");
    QL_REQUIRE(forward > 0.0, "forward (" << forward << ") must be positive");
    QL_REQUIRE(beta >= detail::NoArbSabrModel::beta_min && beta <= detail::NoArbSabrModel::beta_max,
               "beta (" << beta << ") out of bounds");
    Real sigmaI = alpha * std::pow(forward, beta - 1.0);
    QL_REQUIRE(sigmaI >= detail::NoArbSabrModel::sigmaI_min &&
                   sigmaI <= detail::NoArbSabrModel::sigmaI_max,
               "sigmaI = alpha*forward^(beta-1.0) ("
                   << sigmaI << ") out of bounds, alpha=" << alpha
                   << " beta=" << beta << " forward=" << forward);
    QL_REQUIRE(nu >= detail::NoArbSabrModel::nu_min && nu <= detail::NoArbSabrModel::nu_max,
               "nu (" << nu << ") out of bounds");
    QL_REQUIRE(rho >= detail::NoArbSabrModel::rho_min && rho <= detail::NoArbSabrModel::rho_max,
               "rho (" << rho << ") out of bounds");

    // determine a region sufficient for integration in the normal case

    fmin_ = fmax_ = forward_;
    for (Real tmp = p(fmax_);
         tmp > std::max(detail::NoArbSabrModel::i_accuracy / std::max(1.0, fmax_ - fmin_),
                        detail::NoArbSabrModel::density_threshold);
         tmp = p(fmax_)) {
        fmax_ *= 2.0;
    }
    for (Real tmp = p(fmin_);
         tmp > std::max(detail::NoArbSabrModel::i_accuracy / std::max(1.0, fmax_ - fmin_),
                        detail::NoArbSabrModel::density_threshold);
         tmp = p(fmin_)) {
        fmin_ *= 0.5;
    }
    fmin_ = std::max(detail::NoArbSabrModel::strike_min, fmin_);

    QL_REQUIRE(fmax_ > fmin_, "could not find a reasonable integration domain");

    integrator_ =
        ext::make_shared<GaussLobattoIntegral>(
            detail::NoArbSabrModel::i_max_iterations, detail::NoArbSabrModel::i_accuracy);

    detail::D0Interpolator d0(forward_, expiryTime_, alpha_, beta_, nu_, rho_);
    absProb_ = d0();

    try {
        Brent b;
        Real start = std::sqrt(externalForward_ - detail::NoArbSabrModel::strike_min);
        Real tmp =
            b.solve([&](Real x){ return forwardError(x); },
                    detail::NoArbSabrModel::forward_accuracy, start,
                    std::min(detail::NoArbSabrModel::forward_search_step, start / 2.0));
        forward_ = tmp * tmp + detail::NoArbSabrModel::strike_min;
    } catch (Error&) {
        // fall back to unadjusted forward
        forward_ = externalForward_;
    }

    Real d = forwardError(std::sqrt(forward_ - detail::NoArbSabrModel::strike_min));
    numericalForward_ = d + externalForward_;
}

Real NoArbSabrModel::optionPrice(const Real strike) const {
    if (p(std::max(forward_, strike)) < detail::NoArbSabrModel::density_threshold)
        return 0.0;
    return (1.0 - absProb_) *
        ((*integrator_)(integrand(this, strike),
                        strike, std::max(fmax_, 2.0 * strike)) /
            numericalIntegralOverP_);
}

Real NoArbSabrModel::digitalOptionPrice(const Real strike) const {
    if (strike < QL_MIN_POSITIVE_REAL)
        return 1.0;
    if (p(std::max(forward_, strike)) < detail::NoArbSabrModel::density_threshold)
        return 0.0;
    return (1.0 - absProb_)
        * ((*integrator_)(p_integrand(this),
                          strike, std::max(fmax_, 2.0 * strike)) /
           numericalIntegralOverP_);
}

Real NoArbSabrModel::forwardError(const Real forward) const {
    forward_ = forward * forward + detail::NoArbSabrModel::strike_min;
    numericalIntegralOverP_ = (*integrator_)(p_integrand(this),
                                             fmin_, fmax_);
    return optionPrice(0.0) - externalForward_;
}

Real NoArbSabrModel::p(const Real f) const {

    if (f < detail::NoArbSabrModel::density_lower_bound ||
        forward_ < detail::NoArbSabrModel::density_lower_bound)
        return 0.0;

    Real fOmB = std::pow(f, 1.0 - beta_);
    Real FOmB = std::pow(forward_, 1.0 - beta_);

    Real zf = fOmB / (alpha_ * (1.0 - beta_));
    Real zF = FOmB / (alpha_ * (1.0 - beta_));
    Real z = zF - zf;

    // Real JzF = std::sqrt(1.0 - 2.0 * rho_ * nu_ * zF + nu_ * nu_ * zF * zF);
    Real Jmzf = std::sqrt(1.0 + 2.0 * rho_ * nu_ * zf + nu_ * nu_ * zf * zf);
    Real Jz = std::sqrt(1.0 - 2.0 * rho_ * nu_ * z + nu_ * nu_ * z * z);

    Real xz = std::log((Jz - rho_ + nu_ * z) / (1.0 - rho_)) / nu_;
    Real Bp_B = beta_ / FOmB;
    // Real Bpp_B = beta_ * (2.0 * beta_ - 1.0) / (FOmB * FOmB);
    Real kappa1 = 0.125 * nu_ * nu_ * (2.0 - 3.0 * rho_ * rho_) -
                  0.25 * rho_ * nu_ * alpha_ * Bp_B;
    // Real kappa2 = alpha_ * alpha_ * (0.25 * Bpp_B - 0.375 * Bp_B * Bp_B);
    Real gamma = 1.0 / (2.0 * (1.0 - beta_));
    Real sqrtOmR = std::sqrt(1.0 - rho_ * rho_);
    Real h = 0.5 * beta_ * rho_ / ((1.0 - beta_) * Jmzf * Jmzf) *
             (nu_ * zf * std::log(zf * Jz / zF) +
              (1 + rho_ * nu_ * zf) / sqrtOmR *
                  (std::atan((nu_ * z - rho_) / sqrtOmR) +
                   std::atan(rho_ / sqrtOmR)));

    Real res =
        std::pow(Jz, -1.5) / (alpha_ * std::pow(f, beta_) * expiryTime_) *
        std::pow(zf, 1.0 - gamma) * std::pow(zF, gamma) *
        std::exp(-(xz * xz) / (2.0 * expiryTime_) +
                 (h + kappa1 * expiryTime_)) *
        modifiedBesselFunction_i_exponentiallyWeighted(gamma,
                                                       zF * zf / expiryTime_);
    return res;
}

namespace detail {

D0Interpolator::D0Interpolator(const Real forward, const Real expiryTime,
                               const Real alpha, const Real beta, const Real nu,
                               const Real rho)
    : forward_(forward), expiryTime_(expiryTime), alpha_(alpha), beta_(beta),
      nu_(nu), rho_(rho), gamma_(1.0 / (2.0 * (1.0 - beta_))) {

    sigmaI_ = alpha_ * std::pow(forward_, beta_ - 1.0);

    tauG_ = {
        0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5, 2.75, 3.0,
        3.25, 3.5, 3.75, 4.0, 4.25, 4.5, 4.75, 5.0, 5.25, 5.5, 5.75, 6.0, 6.25,
        6.5, 6.75, 7.0, 7.25, 7.5, 7.75, 8.0, 8.25, 8.5, 8.75, 9.0, 9.25, 9.5,
        9.75, 10.0, 10.25, 10.5, 10.75, 11.0, 11.25, 11.5, 11.75, 12.0, 12.25,
        12.5, 12.75, 13.0, 13.25, 13.5, 13.75, 14.0, 14.25, 14.5, 14.75, 15.0,
        15.25, 15.5, 15.75, 16.0, 16.25, 16.5, 16.75, 17.0, 17.25, 17.5, 17.75,
        18.0, 18.25, 18.5, 18.75, 19.0, 19.25, 19.5, 19.75, 20.0, 20.25, 20.5,
        20.75, 21.0, 21.25, 21.5, 21.75, 22.0, 22.25, 22.5, 22.75, 23.0, 23.25,
        23.5, 23.75, 24.0, 24.25, 24.5, 24.75, 25.0, 25.25, 25.5, 25.75, 26.0,
        26.25, 26.5, 26.75, 27.0, 27.25, 27.5, 27.75, 28.0, 28.25, 28.5, 28.75,
        29.0, 29.25, 29.5, 29.75, 30.0
    };

    sigmaIG_ = {
        1.0, 0.8, 0.7, 0.6, 0.5, 0.45, 0.4, 0.35, 0.3, 0.27, 0.24, 0.21,
        0.18, 0.15, 0.125, 0.1, 0.075, 0.05
    };

    rhoG_ = { 0.75, 0.50, 0.25, 0.00, -0.25, -0.50, -0.75 };

    nuG_ = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 };

    betaG_ = { 0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };
}

Real D0Interpolator::operator()() const {

    // we do not need to check the indices here, because this is already
    // done in the NoArbSabr constructor

    const std::array<unsigned long, SabrAbsProbArraySize> & sabrabsprob =
        SabrAbsProbArray();
    Size tauInd = std::upper_bound(tauG_.begin(), tauG_.end(), expiryTime_) -
                                   tauG_.begin();
    if (tauInd == tauG_.size())
        --tauInd; // tau at upper bound
    Real expiryTimeTmp = expiryTime_;
    if (tauInd == 0) {
        ++tauInd;
        expiryTimeTmp = tauG_.front();
    }
    Real tauL = (expiryTimeTmp - tauG_[tauInd - 1]) /
                (tauG_[tauInd] - tauG_[tauInd - 1]);

    Size sigmaIInd =
        sigmaIG_.size() -
        (std::upper_bound(sigmaIG_.rbegin(), sigmaIG_.rend(), sigmaI_) -
         sigmaIG_.rbegin());
    if (sigmaIInd == 0)
        ++sigmaIInd; // sigmaI at upper bound
    Real sigmaIL = (sigmaI_ - sigmaIG_[sigmaIInd - 1]) /
                   (sigmaIG_[sigmaIInd] - sigmaIG_[sigmaIInd - 1]);

    Size rhoInd =
        rhoG_.size() -
        (std::upper_bound(rhoG_.rbegin(), rhoG_.rend(), rho_) - rhoG_.rbegin());
    if (rhoInd == 0) {
        rhoInd++;
    }
    if (rhoInd == rhoG_.size()) {
        rhoInd--;
    }
    Real rhoL =
        (rho_ - rhoG_[rhoInd - 1]) / (rhoG_[rhoInd] - rhoG_[rhoInd - 1]);

    // for nu = 0 we know phi = 0.5*z_F^2
    Size nuInd = std::upper_bound(nuG_.begin(), nuG_.end(), nu_) - nuG_.begin();
    if (nuInd == nuG_.size())
        --nuInd; // nu at upper bound
    Real tmpNuG = nuInd > 0 ? nuG_[nuInd - 1] : 0.0;
    Real nuL = (nu_ - tmpNuG) / (nuG_[nuInd] - tmpNuG);

    // for beta = 1 we know phi = 0.0
    Size betaInd =
        std::upper_bound(betaG_.begin(), betaG_.end(), beta_) - betaG_.begin();
    Real tmpBetaG;
    if (betaInd == betaG_.size())
        tmpBetaG = 1.0;
    else
        tmpBetaG = betaG_[betaInd];
    Real betaL =
        (beta_ - betaG_[betaInd - 1]) / (tmpBetaG - betaG_[betaInd - 1]);

    Real phiRes = 0.0;
    for (int iTau = -1; iTau <= 0; ++iTau) {
        for (int iSigma = -1; iSigma <= 0; ++iSigma) {
            for (int iRho = -1; iRho <= 0; ++iRho) {
                for (int iNu = -1; iNu <= 0; ++iNu) {
                    for (int iBeta = -1; iBeta <= 0; ++iBeta) {
                        Real phiTmp;
                        if (iNu == -1 && nuInd == 0) {
                            phiTmp =
                                0.5 /
                                (sigmaI_ * sigmaI_ * (1.0 - beta_) *
                                 (1.0 - beta_)); // this is 0.5*z_F^2, see above
                        } else {
                            if (iBeta == 0 && betaInd == betaG_.size()) {
                                phiTmp =
                                    phi(detail::NoArbSabrModel::tiny_prob);
                            } else {
                                int ind = (tauInd + iTau +
                                           (sigmaIInd + iSigma +
                                            (rhoInd + iRho +
                                             (nuInd + iNu + ((betaInd + iBeta) *
                                                             nuG_.size())) *
                                                 rhoG_.size()) *
                                                sigmaIG_.size()) *
                                               tauG_.size());
                                QL_REQUIRE(ind >= 0 && ind < 1209600,
                                           "absorption matrix index ("
                                               << ind << ") invalid");
                                phiTmp = phi((Real)sabrabsprob[ind] /
                                             detail::NoArbSabrModel::nsim);
                            }
                        }
                        phiRes += phiTmp * (iTau == -1 ? (1.0 - tauL) : tauL) *
                                  (iSigma == -1 ? (1.0 - sigmaIL) : sigmaIL) *
                                  (iRho == -1 ? (1.0 - rhoL) : rhoL) *
                                  (iNu == -1 ? (1.0 - nuL) : nuL) *
                                  (iBeta == -1 ? (1.0 - betaL) : betaL);
                    }
                }
            }
        }
    }
    return d0(phiRes);
}

Real D0Interpolator::phi(const Real d0) const {
    if (d0 < 1e-14)
        return detail::NoArbSabrModel::phiByTau_cutoff * expiryTime_;
    return boost::math::gamma_q_inv(gamma_, d0) * expiryTime_;
}

Real D0Interpolator::d0(const Real phi) const {
    return boost::math::gamma_q(gamma_, std::max(0.0, phi / expiryTime_));
}

} // namespace detail

} // namespace QuantLib
