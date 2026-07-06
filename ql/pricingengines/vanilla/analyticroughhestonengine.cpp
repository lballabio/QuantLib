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
    // following AnalyticHestonEngine::AP_Helper; the control-variate
    // variance is matched to the model characteristic function at the
    // purely imaginary point i*alpha.
    class AnalyticRoughHestonEngine::AP_Helper {
      public:
        AP_Helper(Time term, Real fwd, Real strike, Real alpha,
                  const AnalyticRoughHestonEngine* enginePtr)
        : term_(term),
          fwd_(fwd),
          strike_(strike),
          alpha_(alpha),
          freq_(std::log(fwd/strike)),
          s_alpha_(std::exp(alpha*freq_)),
          enginePtr_(enginePtr) {
            QL_REQUIRE(enginePtr != nullptr, "pricing engine required");

            const Real moment = enginePtr->chF(
                std::complex<Real>(0.0, alpha_), term).real();
            QL_REQUIRE(moment > 0.0,
                       "invalid characteristic function value; try to "
                       "increase the number of fractional Riccati time steps");
            vAvg_ = -8.0*std::log(moment)/term;
        }

        Real operator()(Real u) const {
            const std::complex<Real> z(u, -alpha_);
            const std::complex<Real> zPrime(u, -alpha_ - 1.0);

            const std::complex<Real> phiBS = std::exp(
                -0.5*vAvg_*term_*(zPrime*zPrime
                    + std::complex<Real>(-zPrime.imag(), zPrime.real())));

            std::complex<Real> phi = enginePtr_->chF(zPrime, term_);

            // the fractional Adams scheme diverges once the oscillation is
            // under-resolved by the time grid (u dt > ~0.35). Along this
            // contour the exact characteristic function satisfies
            // |phi| <= 1 and decays exponentially in u, so diverged
            // far-tail values are truncated to zero.
            if (!std::isfinite(phi.real()) || !std::isfinite(phi.imag())
                    || std::norm(phi) > 1.0)
                phi = std::complex<Real>(0.0);

            return (std::exp(std::complex<Real>(0.0, u*freq_))
                    *(phiBS - phi)/(z*zPrime)
                   ).real()*s_alpha_;
        }

        Real controlVariateValue() const {
            return BlackCalculator(Option::Call, strike_, fwd_,
                                   std::sqrt(vAvg_*term_)).value();
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
        Size integrationOrder, Size timeSteps)
    : GenericModelEngine<RoughHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      timeSteps_(timeSteps),
      integration_(new Integration(
          Integration::gaussLaguerre(integrationOrder))),
      andersenPiterbargEpsilon_(1e-25),
      alpha_(-0.5) {
        QL_REQUIRE(timeSteps > 0, "at least one time step required");
    }

    AnalyticRoughHestonEngine::AnalyticRoughHestonEngine(
        const ext::shared_ptr<RoughHestonModel>& model,
        const Integration& integration,
        Size timeSteps,
        Real andersenPiterbargEpsilon,
        Real alpha)
    : GenericModelEngine<RoughHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      timeSteps_(timeSteps),
      integration_(new Integration(integration)),
      andersenPiterbargEpsilon_(andersenPiterbargEpsilon),
      alpha_(alpha) {
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

        const Real kappa = model_->kappa();
        const Real theta = model_->theta();
        const Real sigma = model_->sigma();
        const Real rho   = model_->rho();
        const Real v0    = model_->v0();
        const Real a     = model_->hurst() + 0.5;

        const std::complex<Real> c0 =
            -0.5*z*(z + std::complex<Real>(0.0, 1.0));
        const std::complex<Real> c1 =
            std::complex<Real>(0.0, rho*sigma)*z - kappa;
        const Real c2 = 0.5*sigma*sigma;

        const std::vector<std::complex<Real>> h =
            FractionalAdams<std::complex<Real>>(a).solve(
                [&](Real, const std::complex<Real>& x)
                    -> std::complex<Real> { return c0 + (c1 + c2*x)*x; },
                std::complex<Real>(0.0), t, timeSteps_);

        const Real dt = t/timeSteps_;

        return kappa*theta*riemannLiouvilleIntegral(h, 1.0, dt)
            + v0*riemannLiouvilleIntegral(h, 1.0 - a, dt);
    }

    std::complex<Real> AnalyticRoughHestonEngine::chF(
        const std::complex<Real>& z, Time t) const {

        const auto key = std::make_tuple(z.real(), z.imag(), t);
        const auto cached = chFCache_.find(key);
        if (cached != chFCache_.end())
            return cached->second;

        const std::complex<Real> value = std::exp(lnChF(z, t));
        chFCache_[key] = value;

        return value;
    }

    Real AnalyticRoughHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        const Date& maturity) const {

        const Real fwd = model_->s0()->value()
            * model_->dividendYield()->discount(maturity)
            / model_->riskFreeRate()->discount(maturity);

        return priceVanillaPayoff(
            payoff, model_->riskFreeRate()->timeFromReference(maturity), fwd);
    }

    Real AnalyticRoughHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Time maturity) const {

        const Real fwd = model_->s0()->value()
            * model_->dividendYield()->discount(maturity)
            / model_->riskFreeRate()->discount(maturity);

        return priceVanillaPayoff(payoff, maturity, fwd);
    }

    Real AnalyticRoughHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Time maturity, Real fwd) const {

        QL_REQUIRE(maturity > 0.0, "maturity must be positive");

        const Real spot = model_->s0()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        const DiscountFactor dr =
            model_->riskFreeRate()->discount(maturity);
        const Real strike = payoff->strike();

        const Real kappa = model_->kappa();
        const Real theta = model_->theta();
        const Real sigma = model_->sigma();
        const Real rho   = model_->rho();
        const Real v0    = model_->v0();
        const Real a     = model_->hurst() + 0.5;

        evaluations_ = 0;

        // decay estimate of the characteristic function along the
        // integration contour; recovers the classical Heston engine value
        // sqrt(1-rho^2)(v0 + kappa theta t)/sigma for alpha = 1
        const Real c_inf = std::sqrt(1.0 - rho*rho)/sigma
            *(v0*std::pow(maturity, 1.0 - a)
                /GammaFunction().value(2.0 - a)
              + kappa*theta*maturity);

        const Real epsilon = andersenPiterbargEpsilon_
            *M_PI/(std::sqrt(strike*fwd)*dr);

        const std::function<Real()> uM = [=]() {
            return Integration::andersenPiterbargIntegrationLimit(
                c_inf, epsilon, v0, maturity);
        };

        const AP_Helper cvHelper(maturity, fwd, strike, alpha_, this);
        const Real cvValue = cvHelper.controlVariateValue();

        const Real scalingFactor = std::max(0.25, std::min(1000.0,
            0.25/std::sqrt(0.5*cvHelper.vAvg()*maturity)));

        const Real h_cv = fwd/M_PI
            *integration_->calculate(c_inf, cvHelper, uM, scalingFactor);
        evaluations_ += integration_->numberOfEvaluations();

        switch (payoff->optionType()) {
          case Option::Call:
            return (cvValue + h_cv)*dr;
          case Option::Put:
            return (cvValue + h_cv - (fwd - strike))*dr;
          default:
            QL_FAIL("unknown option type");
        }
    }

    void AnalyticRoughHestonEngine::calculate() const {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        results_.value =
            priceVanillaPayoff(payoff, arguments_.exercise->lastDate());
    }

    Size AnalyticRoughHestonEngine::numberOfEvaluations() const {
        return evaluations_;
    }
}
