/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/ode/fractionaladams.hpp>
#include <ql/mathconstants.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/analyticroughhestonengine.hpp>

namespace QuantLib {

    // Andersen-Piterbarg integrand with Black-Scholes control variate,
    // following AnalyticHestonEngine::AP_Helper
    class AnalyticRoughHestonEngine::AP_Helper {
      public:
        AP_Helper(Time term, Real fwd, Real strike, Real alpha,
                  const AnalyticRoughHestonEngine* enginePtr)
        : term_(term),
          fwd_(fwd),
          strike_(strike),
          alpha_(alpha),
          freq_(std::log(fwd / strike)),
          s_alpha_(std::exp(alpha * freq_)),
          enginePtr_(enginePtr) {
            QL_REQUIRE(enginePtr != nullptr, "pricing engine required");

            const Real moment{enginePtr->chF(
                std::complex<Real>(0.0, alpha_), term).real()};
            QL_REQUIRE(moment > 0.0,
                       "invalid characteristic function value; try to "
                       "increase the number of fractional Riccati time steps");
            vAvg_ = -8.0 * std::log(moment) / term;
        }

        Real operator()(Real u) const {
            const std::complex<Real> z(u, -alpha_);
            const std::complex<Real> zPrime(u, -alpha_ - 1.0);

            const std::complex<Real> phiBS{std::exp(
                -0.5 * vAvg_ * term_ * (zPrime * zPrime
                    + std::complex<Real>(-zPrime.imag(), zPrime.real())))};

            std::complex<Real> phi{enginePtr_->chF(zPrime, term_)};

            // The fractional Adams scheme diverges once the oscillation is
            // under-resolved by the time grid. The exact characteristic
            //  function satisfies |phi| <= 1 and decays exponentially
            // in u, so far-tail values are truncated to zero.
            if (!std::isfinite(phi.real()) || !std::isfinite(phi.imag())
                    || std::norm(phi) > 1.0)
                phi = std::complex<Real>(0.0);

            return (std::exp(std::complex<Real>(0.0, u * freq_))
                    * (phiBS - phi) / (z * zPrime)
                   ).real() * s_alpha_;
        }

        Real controlVariateValue() const {
            return BlackCalculator(Option::Call, strike_, fwd_,
                                   std::sqrt(vAvg_ * term_)).value();
        }

        Real vAvg() const { return vAvg_; }

      private:
        const Time term_;
        const Real fwd_, strike_, alpha_, freq_, s_alpha_;
        const AnalyticRoughHestonEngine* const enginePtr_;
        Real vAvg_;
    };


    AnalyticRoughHestonEngine::AnalyticRoughHestonEngine(
        const ext::shared_ptr<RoughHestonModel>& model,
        Size integrationOrder, 
        Size timeSteps,
        Approximation approximation)
    : GenericModelEngine<RoughHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      timeSteps_(timeSteps),
      integration_(Integration::gaussLaguerre(integrationOrder)),
      andersenPiterbargEpsilon_(1e-25),
      alpha_(-0.5),
      approximation_(approximation) {
        QL_REQUIRE(timeSteps > 0, "at least one time step required");
    }

    AnalyticRoughHestonEngine::AnalyticRoughHestonEngine(
        const ext::shared_ptr<RoughHestonModel>& model,
        const Integration& integration,
        Size timeSteps,
        Real andersenPiterbargEpsilon,
        Real alpha,
        Approximation approximation)
    : GenericModelEngine<RoughHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      timeSteps_(timeSteps),
      integration_(integration),
      andersenPiterbargEpsilon_(andersenPiterbargEpsilon),
      alpha_(alpha),
      approximation_(approximation) {
        QL_REQUIRE(timeSteps > 0, "at least one time step required");
        QL_REQUIRE(alpha > -1.0 && alpha < 0.0,
                   "alpha (" << alpha << ") must be in (-1, 0)");
    }

    void AnalyticRoughHestonEngine::update() {
        chFCache_.clear();
        GenericModelEngine<RoughHestonModel,
                           VanillaOption::arguments,
                           VanillaOption::results>::update();
    }

    std::complex<Real> AnalyticRoughHestonEngine::lnChF(
        const std::complex<Real>& z, Time t) const {
        return approximation_ == Approximation::Pade
            ? lnChFPade(z, t) : lnChFAdams(z, t);
    }

    // Coefficients of the non-fractional Riccati equation satisfied by the
    // rough Heston h(z, t), shared by the Adams and Padè routes
    AnalyticRoughHestonEngine::RiccatiCoefficients
    AnalyticRoughHestonEngine::riccatiCoefficients(
        const std::complex<Real>& z) const {

        const Real kappa = model_->kappa();
        const Real sigma = model_->sigma();
        const Real rho   = model_->rho();

        const std::complex<Real> i{0.0, 1.0};

        const std::complex<Real> c0{-0.5 * z * (z + i)};
        const std::complex<Real> c1{i * (rho * sigma) * z - kappa};
        const Real c2{0.5 * sigma * sigma};

        return {c0, c1, std::complex<Real>(c2)};
    }

    std::vector<std::complex<Real>> AnalyticRoughHestonEngine::solveAdamsRiccati(
        const std::complex<Real>& z, Time t) const {

        const Real a{model_->hurst() + 0.5};
        const RiccatiCoefficients c{riccatiCoefficients(z)};

        return FractionalAdams<std::complex<Real>>(a).solve(
            [&](Real, const std::complex<Real>& x) -> std::complex<Real> {
                return c.c0 + (c.c1 + c.c2 * x) * x;
            },
            std::complex<Real>(0.0), t, timeSteps_);
    }

    std::complex<Real> AnalyticRoughHestonEngine::lnChFAdams(
        const std::complex<Real>& z, Time t) const {

        const Real kappa = model_->kappa();
        const Real theta = model_->theta();
        const Real v0    = model_->v0();
        const Real a     = model_->hurst() + 0.5;

        const std::vector<std::complex<Real>> h{solveAdamsRiccati(z, t)};

        const Real dt{t / timeSteps_};

        return kappa * theta * riemannLiouvilleIntegral(h, 1.0, dt)
            + v0 * riemannLiouvilleIntegral(h, 1.0 - a, dt);
    }

    std::complex<Real> AnalyticRoughHestonEngine::lnChFPade(
        const std::complex<Real>& z, Time t) const {

        const Real kappa = model_->kappa();
        const Real theta = model_->theta();
        const Real sigma = model_->sigma();
        const Real v0    = model_->v0();
        const Real a     = model_->hurst() + 0.5;

        const PadeCoefficients c{padeCoefficients(z)};

        const Real dt{t / timeSteps_};

        std::vector<std::complex<Real>> h(timeSteps_ + 1);
        for (Size j{0}; j <= timeSteps_; ++j) {
            h[j] = evaluatePade(c, sigma * std::pow(Real(j) * dt, a));
        }

        return kappa * theta * riemannLiouvilleIntegral(h, 1.0, dt)
            + v0 * riemannLiouvilleIntegral(h, 1.0 - a, dt);
    }

    // (3, 3) global rational approximation of Gatheral-Radoicic
    AnalyticRoughHestonEngine::PadeCoefficients
    AnalyticRoughHestonEngine::padeCoefficients(
        const std::complex<Real>& z) const {

        const Real kappa = model_->kappa();
        const Real sigma = model_->sigma();
        const Real rho   = model_->rho();
        const Real a     = model_->hurst() + 0.5;

        const std::complex<Real> i{0.0, 1.0};

        const RiccatiCoefficients rc{riccatiCoefficients(z)};
        const std::complex<Real> c0{rc.c0};
        const std::complex<Real> c1{rc.c1};
        const Real c2{rc.c2.real()};

        // c0 = -z * (z + i) / 2 vanishes exactly at z = 0 and z = -i. There,
        // v = 0 solves the Riccati equation identically, so h is exactly 
        // zero for all t. It also makes the long-time root gamma0 vanish,
        // which degenerates the (3, 3) system.
        if (std::abs(c0) < 1e-14) {
            return {std::complex<Real>(0.0), std::complex<Real>(0.0),
                    std::complex<Real>(0.0), std::complex<Real>(0.0),
                    std::complex<Real>(0.0), std::complex<Real>(0.0)};
        }

        const GammaFunction g;

        // reciprocal gamma 1 / Gamma(x), which vanishes at the poles
        // 0, -1, -2, ...; keeps the long-time coefficients finite as
        // a -> 1
        const auto rGamma
            = [&g](Real x) -> Real { return 1.0 / g.value(x); };

        // Short-time Taylor coefficients beta_k = b_k / sigma^k of
        // h = sum_k b_k t^(k a), rescaled to the variable y = sigma t^a
        const std::complex<Real> b1{c0 * rGamma(1.0 + a)};
        const std::complex<Real> b2{
            c1 * b1 * (g.value(1.0 + a) / g.value(1.0 + 2.0 * a))};
        const std::complex<Real> b3{
            (c1 * b2 + c2 * b1 * b1)
            * (g.value(1.0 + 2.0 * a) / g.value(1.0 + 3.0 * a))};

        const std::complex<Real> beta1{b1 / sigma};
        const std::complex<Real> beta2{b2 / (sigma * sigma)};
        const std::complex<Real> beta3{b3 / (sigma * sigma * sigma)};

        // Long-time coefficients gamma_k = g_k sigma^k: h -> r_- / sigma,
        // the attracting root of the Riccati quadratic, with algebraic
        // corrections in 1 / y.
        const std::complex<Real> w{kappa / sigma - i * (rho * z)};
        const std::complex<Real> A{std::sqrt(z * (z + i) + w * w)};

        // A is the discriminant root separating the two fixed points of the
        // Riccati quadratic; near z where they coincide (A -> 0), gamma_1 and
        // gamma_2 below are ill-conditioned and the (3, 3) approximant is not
        // reliable here.
        QL_REQUIRE(std::abs(A) > 1e-8,
                   "Padè approximation is singular at z = " << z
                   << "; use the Adams "
                   "predictor-corrector engine for this contour point");

        const std::complex<Real> rMinus{w - A};

        const std::complex<Real> gamma0{rMinus / sigma};
        const std::complex<Real> gamma1{-gamma0 * rGamma(1.0 - a) / A};
        const std::complex<Real> gamma2{
            gamma0 * rGamma(1.0 - 2.0 * a) / (A * A)
            + 0.5 * sigma * gamma1 * gamma1 / A};

        // The two matching sets pin down the denominator (q_1, q_2, q_3)
        // through a 3x3 complex linear system. QuantLib::determinant()
        // only accepts a real-valued Matrix, so this closed-form cofactor
        // expansion is used instead of routing through a boost::ublas LU
        // decomposition for what is a fixed, known 3x3 size.
        const auto det3
            = [](const std::complex<Real>& a11, const std::complex<Real>& a12,
                 const std::complex<Real>& a13, const std::complex<Real>& a21,
                 const std::complex<Real>& a22, const std::complex<Real>& a23,
                 const std::complex<Real>& a31, const std::complex<Real>& a32,
                 const std::complex<Real>& a33) -> std::complex<Real> {
            return a11 * (a22 * a33 - a23 * a32)
                 - a12 * (a21 * a33 - a23 * a31)
                 + a13 * (a21 * a32 - a22 * a31);
        };

        const std::complex<Real> det{det3(
            gamma0, gamma1, gamma2, -beta1, gamma0, gamma1, -beta2, -beta1,
            gamma0)};

        QL_REQUIRE(std::abs(det) > 1e-12,
                   "Padè denominator system is ill-conditioned at z = " << z
                   << "; use the Adams predictor-corrector engine for this "
                   "contour point");

        const std::complex<Real> q1{det3(
            beta1, gamma1, gamma2, beta2, gamma0, gamma1, beta3, -beta1,
            gamma0) / det};
        const std::complex<Real> q2{det3(
            gamma0, beta1, gamma2, -beta1, beta2, gamma1, -beta2, beta3,
            gamma0) / det};
        const std::complex<Real> q3{det3(
            gamma0, gamma1, beta1, -beta1, gamma0, beta2, -beta2, -beta1,
            beta3) / det};

        const std::complex<Real> p1{beta1};
        const std::complex<Real> p2{beta2 + beta1 * q1};
        const std::complex<Real> p3{beta3 + beta2 * q1 + beta1 * q2};

        return {p1, p2, p3, q1, q2, q3};
    }

    std::complex<Real> AnalyticRoughHestonEngine::evaluatePade(
        const PadeCoefficients& c, Real y) {
        const std::complex<Real> num{y * (c.p1 + y * (c.p2 + y * c.p3))};
        const std::complex<Real> den{1.0 + y * (c.q1 + y * (c.q2 + y * c.q3))};
        
        return num / den;
    }

    std::complex<Real> AnalyticRoughHestonEngine::padeRiccati(
        const std::complex<Real>& z, Time t) const {
        const Real sigma = model_->sigma();
        const Real a     = model_->hurst() + 0.5;
        
        return evaluatePade(padeCoefficients(z), sigma * std::pow(t, a));
    }

    std::complex<Real> AnalyticRoughHestonEngine::riccatiSolution(
        const std::complex<Real>& z, Time t) const {

        QL_REQUIRE(t > 0.0, "maturity must be positive");

        if (approximation_ == Approximation::Pade) {
            return padeRiccati(z, t);
        }

        return solveAdamsRiccati(z, t).back();
    }

    std::complex<Real> AnalyticRoughHestonEngine::chF(
        const std::complex<Real>& z, Time t) const {

        const auto key{std::make_tuple(z.real(), z.imag(), t)};
        const auto cached{chFCache_.find(key)};
        if (cached != chFCache_.end())
            return cached->second;

        const std::complex<Real> value{std::exp(lnChF(z, t))};
        chFCache_[key] = value;

        return value;
    }

    Real AnalyticRoughHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        const Date& maturity) const {

        const Real fwd{model_->s0()->value()
            * model_->dividendYield()->discount(maturity)
            / model_->riskFreeRate()->discount(maturity)};

        return priceVanillaPayoff(
            payoff, model_->riskFreeRate()->timeFromReference(maturity), fwd);
    }

    Real AnalyticRoughHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Time maturity) const {

        QL_REQUIRE(maturity > 0.0, "maturity must be positive");

        const Real fwd{model_->s0()->value()
            * model_->dividendYield()->discount(maturity)
            / model_->riskFreeRate()->discount(maturity)};

        return priceVanillaPayoff(payoff, maturity, fwd);
    }

    Real AnalyticRoughHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Time maturity, Real fwd) const {

        QL_REQUIRE(maturity > 0.0, "maturity must be positive");

        const Real spot{model_->s0()->value()};
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        const DiscountFactor dr{model_->riskFreeRate()->discount(maturity)};
        const Real strike{payoff->strike()};

        const Real kappa{model_->kappa()};
        const Real theta{model_->theta()};
        const Real sigma{model_->sigma()};
        const Real rho{model_->rho()};
        const Real v0{model_->v0()};
        const Real a{model_->hurst() + 0.5};

        evaluations_ = 0;

        // decay estimate of the characteristic function along the
        // integration contour; recovers the classical Heston engine value
        // sqrt(1 - rho ^ 2)(v0 + kappa theta t) / sigma for alpha = 1
        // TODO: the a < 1 rescaling is a heuristic, not a derived asymptotic;
        // revisit with a proper derivation or a truncation stress test.
        const Real c_inf{std::sqrt(1.0 - rho * rho) / sigma
            * (v0 * std::pow(maturity, 1.0 - a)
                / GammaFunction().value(2.0 - a)
              + kappa * theta * maturity)};

        const Real epsilon{andersenPiterbargEpsilon_
            * M_PI / (std::sqrt(strike * fwd) * dr)};

        const std::function<Real()> uM{[=]() {
            return Integration::andersenPiterbargIntegrationLimit(
                c_inf, epsilon, v0, maturity);
        }};

        const AP_Helper cvHelper(maturity, fwd, strike, alpha_, this);

        const Real cvValue{cvHelper.controlVariateValue()};

        const Real scalingFactor{std::max(0.25, std::min(1000.0,
            0.25 / std::sqrt(0.5 * cvHelper.vAvg() * maturity)))};

        const Real h_cv{fwd / M_PI
            * integration_.calculate(c_inf, cvHelper, uM, scalingFactor)};
        evaluations_ += integration_.numberOfEvaluations();

        switch (payoff->optionType()) {
          case Option::Call:
            return (cvValue + h_cv) * dr;
          case Option::Put:
            return (cvValue + h_cv - (fwd - strike)) * dr;
          default:
            QL_FAIL("unknown option type");
        }
    }

    void AnalyticRoughHestonEngine::calculate() const {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        const ext::shared_ptr<PlainVanillaPayoff> payoff{
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff)};
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        results_.value =
            priceVanillaPayoff(payoff, arguments_.exercise->lastDate());
    }

    Size AnalyticRoughHestonEngine::numberOfEvaluations() const {
        return evaluations_;
    }
}
