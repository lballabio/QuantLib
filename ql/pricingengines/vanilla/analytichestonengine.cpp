/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file hestonmodel.hpp
  \brief analytic pricing engine for a heston option
  based on fourier transformation
*/

#include <ql/functional.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/integrals/exponentialintegrals.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/integrals/expsinhintegral.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/expm1.hpp>
#include <ql/math/functional.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>

#include <boost/math/tools/minima.hpp>
#include <boost/math/special_functions/sign.hpp>

#include <cmath>
#include <limits>
#include <utility>

#if defined(QL_PATCH_MSVC)
#pragma warning(disable: 4180)
#endif

#include <iostream>

namespace QuantLib {

    namespace {

        class integrand1 {
          private:
            const Real c_inf_;
            const ext::function<Real(Real)> f_;
          public:
            integrand1(Real c_inf, ext::function<Real(Real)> f) : c_inf_(c_inf), f_(std::move(f)) {}
            Real operator()(Real x) const {
                if ((1.0-x)*c_inf_ > QL_EPSILON)
                    return f_(-std::log(0.5-0.5*x)/c_inf_)/((1.0-x)*c_inf_);
                else
                    return 0.0;
            }
        };

        class integrand2 {
          private:
            const Real c_inf_;
            const ext::function<Real(Real)> f_;
          public:
            integrand2(Real c_inf, ext::function<Real(Real)> f) : c_inf_(c_inf), f_(std::move(f)) {}
            Real operator()(Real x) const {
                if (x*c_inf_ > QL_EPSILON) {
                    return f_(-std::log(x)/c_inf_)/(x*c_inf_);
                } else {
                    return 0.0;
                }
            }
        };

        class integrand3 {
          private:
            const integrand2 int_;
          public:
            integrand3(Real c_inf, const ext::function<Real(Real)>& f)
            : int_(c_inf, f) {}

            Real operator()(Real x) const { return int_(1.0-x); }
        };

        class u_Max {
          public:
            u_Max(Real c_inf, Real epsilon) : c_inf_(c_inf), logEpsilon_(std::log(epsilon)) {}

            Real operator()(Real u) const {
                ++evaluations_;
                return c_inf_*u + std::log(u) + logEpsilon_;
            }

            Size evaluations() const { return evaluations_; }

          private:
            const Real c_inf_, logEpsilon_;
            mutable Size evaluations_ = 0;
        };


        class uHat_Max {
          public:
            uHat_Max(Real v0T2, Real epsilon) : v0T2_(v0T2), logEpsilon_(std::log(epsilon)) {}

            Real operator()(Real u) const {
                ++evaluations_;
                return v0T2_*u*u + std::log(u) + logEpsilon_;
            }

            Size evaluations() const { return evaluations_; }

          private:
            const Real v0T2_, logEpsilon_;
            mutable Size evaluations_ = 0;
        };
    }

    // helper class for integration
    class AnalyticHestonEngine::Fj_Helper {
    public:
      Fj_Helper(Real kappa, Real theta, Real sigma, Real v0,
                Real s0, Real rho,
                const AnalyticHestonEngine* engine,
                ComplexLogFormula cpxLog,
                Time term, Real strike, Real ratio, Size j);

      Real operator()(Real phi) const;

    private:
        const Size j_;
        //     const VanillaOption::arguments& arg_;
        const Real kappa_, theta_, sigma_, v0_;
        const ComplexLogFormula cpxLog_;

        // helper variables
        const Time term_;
        const Real x_, sx_, dd_;
        const Real sigma2_, rsigma_;
        const Real t0_;

        // log branch counter
        mutable int  b_;     // log branch counter
        mutable Real g_km1_; // imag part of last log value

        const AnalyticHestonEngine* const engine_;
    };


    AnalyticHestonEngine::Fj_Helper::Fj_Helper(Real kappa, Real theta,
        Real sigma, Real v0, Real s0, Real rho,
        const AnalyticHestonEngine* const engine,
        ComplexLogFormula cpxLog,
        Time term, Real strike, Real ratio, Size j)
        :
        j_(j),
        kappa_(kappa),
        theta_(theta),
        sigma_(sigma),
        v0_(v0),
        cpxLog_(cpxLog),
        term_(term),
        x_(std::log(s0)),
        sx_(std::log(strike)),
        dd_(x_-std::log(ratio)),
        sigma2_(sigma_*sigma_),
        rsigma_(rho*sigma_),
        t0_(kappa - ((j== 1)? rho*sigma : Real(0))),
        b_(0),
        g_km1_(0),
        engine_(engine)
    {
    }

    Real AnalyticHestonEngine::Fj_Helper::operator()(Real phi) const
    {
        const Real rpsig(rsigma_*phi);

        const std::complex<Real> t1 = t0_+std::complex<Real>(0, -rpsig);
        const std::complex<Real> d =
            std::sqrt(t1*t1 - sigma2_*phi
                      *std::complex<Real>(-phi, (j_== 1)? 1 : -1));
        const std::complex<Real> ex = std::exp(-d*term_);
        const std::complex<Real> addOnTerm =
            engine_ != nullptr ? engine_->addOnTerm(phi, term_, j_) : Real(0.0);

        if (cpxLog_ == Gatheral) {
            if (phi != 0.0) {
                if (sigma_ > 1e-5) {
                    const std::complex<Real> p = (t1-d)/(t1+d);
                    const std::complex<Real> g
                                            = std::log((1.0 - p*ex)/(1.0 - p));

                    return
                        std::exp(v0_*(t1-d)*(1.0-ex)/(sigma2_*(1.0-ex*p))
                                 + (kappa_*theta_)/sigma2_*((t1-d)*term_-2.0*g)
                                 + std::complex<Real>(0.0, phi*(dd_-sx_))
                                 + addOnTerm
                                 ).imag()/phi;
                }
                else {
                    const std::complex<Real> td = phi/(2.0*t1)
                                   *std::complex<Real>(-phi, (j_== 1)? 1 : -1);
                    const std::complex<Real> p = td*sigma2_/(t1+d);
                    const std::complex<Real> g = p*(1.0-ex);

                    return
                        std::exp(v0_*td*(1.0-ex)/(1.0-p*ex)
                                 + (kappa_*theta_)*(td*term_-2.0*g/sigma2_)
                                 + std::complex<Real>(0.0, phi*(dd_-sx_))
                                 + addOnTerm
                                 ).imag()/phi;
                }
            }
            else {
                // use l'Hospital's rule to get lim_{phi->0}
                if (j_ == 1) {
                    const Real kmr = rsigma_-kappa_;
                    if (std::fabs(kmr) > 1e-7) {
                        return dd_-sx_
                            + (std::exp(kmr*term_)*kappa_*theta_
                               -kappa_*theta_*(kmr*term_+1.0) ) / (2*kmr*kmr)
                            - v0_*(1.0-std::exp(kmr*term_)) / (2.0*kmr);
                    }
                    else
                        // \kappa = \rho * \sigma
                        return dd_-sx_ + 0.25*kappa_*theta_*term_*term_
                                       + 0.5*v0_*term_;
                }
                else {
                    return dd_-sx_
                        - (std::exp(-kappa_*term_)*kappa_*theta_
                           +kappa_*theta_*(kappa_*term_-1.0))/(2*kappa_*kappa_)
                        - v0_*(1.0-std::exp(-kappa_*term_))/(2*kappa_);
                }
            }
        }
        else if (cpxLog_ == BranchCorrection) {
            const std::complex<Real> p = (t1+d)/(t1-d);

            // next term: g = std::log((1.0 - p*std::exp(d*term_))/(1.0 - p))
            std::complex<Real> g;

            // the exp of the following expression is needed.
            const std::complex<Real> e = std::log(p)+d*term_;

            // does it fit to the machine precision?
            if (std::exp(-e.real()) > QL_EPSILON) {
                g = std::log((1.0 - p/ex)/(1.0 - p));
            } else {
                // use a "big phi" approximation
                g = d*term_ + std::log(p/(p - 1.0));

                if (g.imag() > M_PI || g.imag() <= -M_PI) {
                    // get back to principal branch of the complex logarithm
                    Real im = std::fmod(g.imag(), 2*M_PI);
                    if (im > M_PI)
                        im -= 2*M_PI;
                    else if (im <= -M_PI)
                        im += 2*M_PI;

                    g = std::complex<Real>(g.real(), im);
                }
            }

            // be careful here as we have to use a log branch correction
            // to deal with the discontinuities of the complex logarithm.
            // the principal branch is not always the correct one.
            // (s. A. Sepp, chapter 4)
            // remark: there is still the change that we miss a branch
            // if the order of the integration is not high enough.
            const Real tmp = g.imag() - g_km1_;
            if (tmp <= -M_PI)
                ++b_;
            else if (tmp > M_PI)
                --b_;

            g_km1_ = g.imag();
            g += std::complex<Real>(0, 2*b_*M_PI);

            return std::exp(v0_*(t1+d)*(ex-1.0)/(sigma2_*(ex-p))
                            + (kappa_*theta_)/sigma2_*((t1+d)*term_-2.0*g)
                            + std::complex<Real>(0,phi*(dd_-sx_))
                            + addOnTerm
                            ).imag()/phi;
        }
        else {
            QL_FAIL("unknown complex logarithm formula");
        }
    }

    AnalyticHestonEngine::OptimalAlpha::OptimalAlpha(
        const Time t,
        const AnalyticHestonEngine* const enginePtr)
    : t_(t),
      fwd_(enginePtr->model_->process()->s0()->value()
              * enginePtr->model_->process()->dividendYield()->discount(t)
              / enginePtr->model_->process()->riskFreeRate()->discount(t)),
      kappa_(enginePtr->model_->kappa()),
      theta_(enginePtr->model_->theta()),
      sigma_(enginePtr->model_->sigma()),
      rho_(enginePtr->model_->rho()),
      bits_(int(0.5*std::numeric_limits<Real>::digits)),
      eps_(std::pow(2, -bits_)),
      enginePtr_(enginePtr),
      evaluations_(0) {
        km_ = k(0.0, -1);
        kp_ = k(0.0,  1);
    }

    Real AnalyticHestonEngine::OptimalAlpha::alphaMax(Real strike) const {
        const Real eps = 1e-8;
        const auto cm = [this](Real k) -> Real { return M(k) - t_; };

        Real alpha_max;
        const Real adx = kappa_ - sigma_*rho_;
        if (adx > 0.0) {
            const Real kp_2pi = k(2*M_PI, 1);

            alpha_max = Brent().solve(
                cm, eps_, 0.5*(kp_ + kp_2pi), (1+eps)*kp_, (1-eps)*kp_2pi
            ) - 1.0;
        }
        else if (adx < 0.0) {
            const Time tCut = -2/(kappa_ - sigma_*rho_*kp_);
            if (t_ < tCut) {
                const Real kp_pi = k(M_PI, 1);
                alpha_max = Brent().solve(
                    cm, eps_, 0.5*(kp_ + kp_pi), (1+eps)*kp_, (1-eps)*kp_pi
                ) - 1.0;
            }
            else {
                alpha_max = Brent().solve(
                    cm, eps_, 0.5*(1.0 + kp_),
                    1 + eps, (1-eps)*kp_
                ) - 1.0;
            }
        }
        else { // adx == 0.0
            const Real kp_pi = k(M_PI, 1);
            alpha_max = Brent().solve(
                cm, eps_, 0.5*(kp_ + kp_pi), (1+eps)*kp_, (1-eps)*kp_pi
            ) - 1.0;
        }

        QL_REQUIRE(alpha_max >= 0.0, "alpha max must be larger than zero");

        return alpha_max;
    }

    std::pair<Real, Real>
    AnalyticHestonEngine::OptimalAlpha::alphaGreaterZero(Real strike) const {
        const Real alpha_max = alphaMax(strike);

        return findMinima(eps_, std::max(2*eps_, (1.0-1e-6)*alpha_max), strike);
    }

    Real AnalyticHestonEngine::OptimalAlpha::alphaMin(Real strike) const {
        const auto cm = [this](Real k) -> Real { return M(k) - t_; };

        const Real km_2pi = k(2*M_PI, -1);

        const Real alpha_min = Brent().solve(
            cm, eps_, 0.5*(km_2pi + km_), (1-1e-8)*km_2pi, (1+1e-8)*km_
        ) - 1.0;

        QL_REQUIRE(alpha_min <= -1.0, "alpha min must be smaller than minus one");

        return alpha_min;
    }

    std::pair<Real, Real>
    AnalyticHestonEngine::OptimalAlpha::alphaSmallerMinusOne(Real strike) const {
        const Real alpha_min = alphaMin(strike);

        return findMinima(
            std::min(-1.0-1e-6, -1.0 + (1.0-1e-6)*(alpha_min + 1.0)),
            -1.0 - eps_, strike
        );
    }

    Real AnalyticHestonEngine::OptimalAlpha::operator()(Real strike) const {
        try {
            const std::pair<Real, Real> minusOne = alphaSmallerMinusOne(strike);
            const std::pair<Real, Real> greaterZero = alphaGreaterZero(strike);

            if (minusOne.second < greaterZero.second) {
                return minusOne.first;
            }
            else {
                return greaterZero.first;
            }
//            if (fwd_ >= strike) {
//                return std::min(-1-1e-6, alphaSmallerMinusOne(strike).first);
//            }
//            else {
//                return std::max(1e-6, alphaGreaterZero(strike).first);
//            }
        }
        catch (const Error&) {
            return -0.5;
        }

    }

    Size AnalyticHestonEngine::OptimalAlpha::numberOfEvaluations() const {
        return evaluations_;
    }

    std::pair<Real, Real> AnalyticHestonEngine::OptimalAlpha::findMinima(
        Real lower, Real upper, Real strike) const {
        const Real freq = std::log(fwd_/strike);

        return boost::math::tools::brent_find_minima(
            [freq, this](Real alpha) -> Real {
                ++evaluations_;
                const std::complex<Real> z(0, -(alpha+1));
                return enginePtr_->lnChF(z, t_).real()
                    - std::log(alpha*(alpha+1)) + alpha*freq;
            },
            lower, upper, bits_
        );
    }

    Real AnalyticHestonEngine::OptimalAlpha::M(Real k) const {
        const Real beta = kappa_ - sigma_*rho_*k;

        if (k >= km_ && k <= kp_) {
            const Real D = std::sqrt(beta*beta - sigma_*sigma_*k*(k-1));
            return std::log((beta-D)/(beta+D)) / D;
        }
        else {
            const Real D_imag =
                std::sqrt(-(beta*beta - sigma_*sigma_*k*(k-1)));

            return 2/D_imag
                * ( ((beta>0.0)? M_PI : 0.0) - std::atan(D_imag/beta) );
        }
    }

    Real AnalyticHestonEngine::OptimalAlpha::k(Real x, Integer sgn) const {
        return ( (sigma_ - 2*rho_*kappa_)
                + sgn*std::sqrt(
                      squared(sigma_-2*rho_*kappa_)
                    + 4*(kappa_*kappa_ + x*x/(t_*t_))*(1-rho_*rho_)))
               /(2*sigma_*(1-rho_*rho_));
    }

    AnalyticHestonEngine::AP_Helper::AP_Helper(
        Time term, Real fwd, Real strike, ComplexLogFormula cpxLog,
        const AnalyticHestonEngine* const enginePtr,
        const Real alpha)
    : term_(term),
      fwd_(fwd),
      strike_(strike),
      freq_(std::log(fwd/strike)),
      cpxLog_(cpxLog),
      enginePtr_(enginePtr),
      alpha_(alpha),
      s_alpha_(std::exp(alpha*freq_))
      {
        QL_REQUIRE(enginePtr != nullptr, "pricing engine required");

        const Real v0    = enginePtr->model_->v0();
        const Real kappa = enginePtr->model_->kappa();
        const Real theta = enginePtr->model_->theta();
        const Real sigma = enginePtr->model_->sigma();
        const Real rho   = enginePtr->model_->rho();

        switch(cpxLog_) {
          case AndersenPiterbarg:
              vAvg_ = (1-std::exp(-kappa*term))*(v0 - theta)
                        /(kappa*term) + theta;
            break;
          case AndersenPiterbargOptCV:
              vAvg_ = -8.0*std::log(enginePtr->chF(
                         std::complex<Real>(0, alpha_), term).real())/term;
            break;
          case AsymptoticChF:
            phi_ = -(v0+term*kappa*theta)/sigma
                * std::complex<Real>(std::sqrt(1-rho*rho), rho);

            psi_ = std::complex<Real>(
                (kappa- 0.5*rho*sigma)*(v0 + term*kappa*theta)
                + kappa*theta*std::log(4*(1-rho*rho)),
                - ((0.5*rho*rho*sigma - kappa*rho)/std::sqrt(1-rho*rho)
                        *(v0 + kappa*theta*term)
                  - 2*kappa*theta*std::atan(rho/std::sqrt(1-rho*rho))))
                          /(sigma*sigma);
            break;
          case AngledContour:
          case AngledContourNoCV:
            {
                vAvg_ = (1-std::exp(-kappa*term))*(v0 - theta)
                          /(kappa*term) + theta;

                const Real r = rho - sigma*freq_ / (v0 + kappa*theta*term);
                tanPhi_ = std::tan(
                     (r*freq_ < 0)? M_PI/12*boost::math::sign(freq_) : 0.0
                );
            }
            break;
          default:
            QL_FAIL("unknown control variate");
        }
    }

    Real AnalyticHestonEngine::AP_Helper::operator()(Real u) const {
        QL_REQUIRE(   enginePtr_->addOnTerm(u, term_, 1)
                        == std::complex<Real>(0.0)
                   && enginePtr_->addOnTerm(u, term_, 2)
                        == std::complex<Real>(0.0),
                   "only Heston model is supported");

        std::complex<Real> phiBS;
        constexpr std::complex<Real> i(0, 1);

        if (cpxLog_ == AngledContour || cpxLog_ == AngledContourNoCV) {
            const std::complex<Real> h_u(u, u*tanPhi_ - alpha_);
            const std::complex<Real> hPrime(h_u-i);

            phiBS = (cpxLog_ == AngledContour)
                ? std::exp(
                    -0.5*vAvg_*term_*(hPrime*hPrime +
                            std::complex<Real>(-hPrime.imag(), hPrime.real())))
                : 0.0;

            return std::exp(-u*tanPhi_*freq_)
                    *(std::exp(std::complex<Real>(0.0, u*freq_))
                      *std::complex<Real>(1, tanPhi_)
                      *(phiBS - enginePtr_->chF(hPrime, term_))/(h_u*hPrime)
                      ).real()*s_alpha_;
        }

        const std::complex<Real> z(u, -alpha_);
        const std::complex<Real> zPrime(u, -alpha_-1);

        switch (cpxLog_) {
          case AndersenPiterbarg:
          case AndersenPiterbargOptCV:
            phiBS = std::exp(
                -0.5*vAvg_*term_*(zPrime*zPrime +
                        std::complex<Real>(-zPrime.imag(), zPrime.real()))
            );
            break;
          case AsymptoticChF:
            phiBS = std::exp(u*phi_ + psi_);
            break;
          default:
            QL_FAIL("unknown control variate");
        }

        return (std::exp(std::complex<Real> (0.0, u*freq_))
            * (phiBS - enginePtr_->chF(zPrime, term_)) / (z*zPrime)
            ).real()*s_alpha_;
    }

    Real AnalyticHestonEngine::AP_Helper::controlVariateValue() const {
        if (   cpxLog_ == AngledContour
            || cpxLog_ == AndersenPiterbarg || cpxLog_ == AndersenPiterbargOptCV) {
              return BlackCalculator(
                  Option::Call, strike_, fwd_, std::sqrt(vAvg_*term_))
                      .value();
        }
        else if (cpxLog_ == AsymptoticChF) {
            QL_REQUIRE(alpha_ == -0.5, "alpha must be equal to -0.5");

            const std::complex<Real> phiFreq(phi_.real(), phi_.imag() + freq_);

            using namespace ExponentialIntegral;
            return fwd_ - std::sqrt(strike_*fwd_)/M_PI*
                (std::exp(psi_)*(
                      -2.0*Ci(-0.5*phiFreq)*std::sin(0.5*phiFreq)
                       +std::cos(0.5*phiFreq)*(M_PI+2.0*Si(0.5*phiFreq)))).real();
        }
        else if (cpxLog_ == AngledContourNoCV) {
            return     ((alpha_ <=  0.0)? fwd_ : 0.0)
                  -    ((alpha_ <= -1.0)? strike_ : 0.0)
                  -0.5*((alpha_ ==  0.0)? fwd_ :0.0)
                  +0.5*((alpha_ == -1.0)? strike_: 0.0);
        }
        else
            QL_FAIL("unknown control variate");
    }

    std::complex<Real> AnalyticHestonEngine::chF(
        const std::complex<Real>& z, Time t) const {
        if (model_->sigma() > 1e-6 || model_->kappa() < 1e-8) {
            return std::exp(lnChF(z, t));
        }
        else {

            // ToDo: expansion in lnChF and then exp(...)
            const Real kappa = model_->kappa();
            const Real sigma = model_->sigma();
            const Real theta = model_->theta();
            const Real rho   = model_->rho();
            const Real v0    = model_->v0();

            const Real sigma2 = sigma*sigma;

            const Real kt = kappa*t;
            const Real ekt = std::exp(kt);
            const Real e2kt = std::exp(2*kt);
            const Real rho2 = rho*rho;
            const std::complex<Real> zpi = z + std::complex<Real>(0.0, 1.0);

            return std::exp(-(((theta - v0 + ekt*((-1 + kt)*theta + v0))
                    *z*zpi)/ekt)/(2.*kappa))

                + (std::exp(-(kt) - ((theta - v0 + ekt
                    *((-1 + kt)*theta + v0))*z*zpi)
                /(2.*ekt*kappa))*rho*(2*theta + kt*theta -
                    v0 - kt*v0 + ekt*((-2 + kt)*theta + v0))
                *(1.0 - std::complex<Real>(-z.imag(),z.real()))*z*z)
                    /(2.*kappa*kappa)*sigma

                   + (std::exp(-2*kt - ((theta - v0 + ekt
                *((-1 + kt)*theta + v0))*z*zpi)/(2.*ekt*kappa))*z*z*zpi
                *(-2*rho2*squared(2*theta + kt*theta - v0 -
                    kt*v0 + ekt*((-2 + kt)*theta + v0))
                  *z*z*zpi + 2*kappa*v0*(-zpi
                    + e2kt*(zpi + 4*rho2*z) - 2*ekt*(2*rho2*z
                    + kt*(zpi + rho2*(2 + kt)*z))) + kappa*theta*(zpi + e2kt
                *(-5.0*zpi - 24*rho2*z+ 2*kt*(zpi + 4*rho2*z)) +
                4*ekt*(zpi + 6*rho2*z + kt*(zpi + rho2*(4 + kt)*z)))))
                /(16.*squared(squared(kappa)))*sigma2;
        }
    }

    std::complex<Real> AnalyticHestonEngine::lnChF(
        const std::complex<Real>& z, Time t) const {

        const Real kappa = model_->kappa();
        const Real sigma = model_->sigma();
        const Real theta = model_->theta();
        const Real rho   = model_->rho();
        const Real v0    = model_->v0();

        const Real sigma2 = sigma*sigma;

        const std::complex<Real> g
            = kappa + rho*sigma*std::complex<Real>(z.imag(), -z.real());

        const std::complex<Real> D = std::sqrt(
            g*g + (z*z + std::complex<Real>(-z.imag(), z.real()))*sigma2);

        // reduce cancelation errors, see. L. Andersen and M. Lake
        std::complex<Real> r(g-D);
        if (g.real()*D.real() + g.imag()*D.imag() > 0.0) {
            r = -sigma2*z*std::complex<Real>(z.real(), z.imag()+1)/(g+D);
        }

        std::complex<Real> y;
        if (D.real() != 0.0 || D.imag() != 0.0) {
            y = expm1(-D*t)/(2.0*D);
        }
        else
            y = -0.5*t;

        const std::complex<Real> A
            = kappa*theta/sigma2*(r*t - 2.0*log1p(-r*y));
        const std::complex<Real> B
            = z*std::complex<Real>(z.real(), z.imag()+1)*y/(1.0-r*y);

        return A+v0*B;
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const ext::shared_ptr<HestonModel>& model,
                              Size integrationOrder)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_     (Gatheral),
      integration_(new Integration(
                          Integration::gaussLaguerre(integrationOrder))),
      andersenPiterbargEpsilon_(Null<Real>()),
      alpha_(-0.5) {
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const ext::shared_ptr<HestonModel>& model,
                              Real relTolerance, Size maxEvaluations)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(Gatheral),
      integration_(new Integration(Integration::gaussLobatto(
                              relTolerance, Null<Real>(), maxEvaluations))),
      andersenPiterbargEpsilon_(Null<Real>()),
      alpha_(-0.5) {
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const ext::shared_ptr<HestonModel>& model,
                              ComplexLogFormula cpxLog,
                              const Integration& integration,
                              Real andersenPiterbargEpsilon,
                              Real alpha)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(cpxLog),
      integration_(new Integration(integration)),
      andersenPiterbargEpsilon_(andersenPiterbargEpsilon),
      alpha_(alpha) {
        QL_REQUIRE(   cpxLog_ != BranchCorrection
                   || !integration.isAdaptiveIntegration(),
                   "Branch correction does not work in conjunction "
                   "with adaptive integration methods");
    }

    AnalyticHestonEngine::ComplexLogFormula
        AnalyticHestonEngine::optimalControlVariate(
        Time t, Real v0, Real kappa, Real theta, Real sigma, Real rho) {

        if (t > 0.15 && (v0+t*kappa*theta)/sigma*std::sqrt(1-rho*rho) < 0.15
                && ((kappa- 0.5*rho*sigma)*(v0 + t*kappa*theta)
                    + kappa*theta*std::log(4*(1-rho*rho)))/(sigma*sigma) < 0.5) {
            return AsymptoticChF;
        }
        else {
            return AngledContour;
        }
    }

    Size AnalyticHestonEngine::numberOfEvaluations() const {
        return evaluations_;
    }

    Real AnalyticHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        const Date& maturity) const {

        const ext::shared_ptr<HestonProcess>& process = model_->process();
        const Real fwd = process->s0()->value()
             * process->dividendYield()->discount(maturity)
             / process->riskFreeRate()->discount(maturity);

        return priceVanillaPayoff(payoff, process->time(maturity), fwd);
    }

    Real AnalyticHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff, Time maturity) const {

        const ext::shared_ptr<HestonProcess>& process = model_->process();
        const Real fwd = process->s0()->value()
             * process->dividendYield()->discount(maturity)
             / process->riskFreeRate()->discount(maturity);

        return priceVanillaPayoff(payoff, maturity, fwd);
    }

    Real AnalyticHestonEngine::priceVanillaPayoff(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Time maturity, Real fwd) const {

        Real value;

        const ext::shared_ptr<HestonProcess>& process = model_->process();
        const DiscountFactor dr = process->riskFreeRate()->discount(maturity);

        const Real strike = payoff->strike();
        const Real spot = process->s0()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        const DiscountFactor df = spot/fwd;
        const DiscountFactor dd = dr/df;

        const Real kappa = model_->kappa();
        const Real sigma = model_->sigma();
        const Real theta = model_->theta();
        const Real rho   = model_->rho();
        const Real v0    = model_->v0();

        evaluations_ = 0;

        switch(cpxLog_) {
          case Gatheral:
          case BranchCorrection: {
            const Real c_inf = std::min(0.2, std::max(0.0001,
                std::sqrt(1.0-rho*rho)/sigma))*(v0 + kappa*theta*maturity);

            const Real p1 = integration_->calculate(c_inf,
                Fj_Helper(kappa, theta, sigma, v0, spot, rho, this,
                          cpxLog_, maturity, strike, df, 1))/M_PI;
            evaluations_ += integration_->numberOfEvaluations();

            const Real p2 = integration_->calculate(c_inf,
                Fj_Helper(kappa, theta, sigma, v0, spot, rho, this,
                          cpxLog_, maturity, strike, df, 2))/M_PI;
            evaluations_ += integration_->numberOfEvaluations();

            switch (payoff->optionType())
            {
              case Option::Call:
                value = spot*dd*(p1+0.5)
                               - strike*dr*(p2+0.5);
                break;
              case Option::Put:
                value = spot*dd*(p1-0.5)
                               - strike*dr*(p2-0.5);
                break;
              default:
                QL_FAIL("unknown option type");
            }
          }
          break;
          case AndersenPiterbarg:
          case AndersenPiterbargOptCV:
          case AsymptoticChF:
          case AngledContour:
          case AngledContourNoCV:
          case OptimalCV: {
            const Real c_inf =
                std::sqrt(1.0-rho*rho)*(v0 + kappa*theta*maturity)/sigma;

            const Real epsilon = andersenPiterbargEpsilon_
                *M_PI/(std::sqrt(strike*fwd)*dr);

            const ext::function<Real()> uM = [&](){
                return Integration::andersenPiterbargIntegrationLimit(
                    c_inf, epsilon, v0, maturity);
            };

            const ComplexLogFormula finalLog = (cpxLog_ == OptimalCV)
                ? optimalControlVariate(maturity, v0, kappa, theta, sigma, rho)
                : cpxLog_;

            const AP_Helper cvHelper(
                 maturity, fwd, strike, finalLog, this, alpha_
            );

            const Real cvValue = cvHelper.controlVariateValue();

            const Real vAvg = (1-std::exp(-kappa*maturity))*(v0-theta)/(kappa*maturity) + theta;

            const Real scalingFactor
                = std::max(0.001, std::min(1000.0, 0.25/std::sqrt(0.5*vAvg*maturity)));

            const Real h_cv =
                fwd/M_PI*integration_->calculate(c_inf, cvHelper, uM, scalingFactor);

            evaluations_ += integration_->numberOfEvaluations();

            switch (payoff->optionType())
            {
              case Option::Call:
                value = (cvValue + h_cv)*dr;
                break;
              case Option::Put:
                value = (cvValue + h_cv - (fwd - strike))*dr;
                break;
              default:
                QL_FAIL("unknown option type");
            }
          }
          break;
          default:
            QL_FAIL("unknown complex log formula");
        }

        return value;
    }

    void AnalyticHestonEngine::calculate() const
    {
        // this is a european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // plain vanilla
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        const Date exerciseDate = arguments_.exercise->lastDate();

        results_.value = priceVanillaPayoff(payoff, exerciseDate);
    }


    AnalyticHestonEngine::Integration::Integration(Algorithm intAlgo,
                                                   ext::shared_ptr<Integrator> integrator)
    : intAlgo_(intAlgo), integrator_(std::move(integrator)) {}

    AnalyticHestonEngine::Integration::Integration(
        Algorithm intAlgo, ext::shared_ptr<GaussianQuadrature> gaussianQuadrature)
    : intAlgo_(intAlgo), gaussianQuadrature_(std::move(gaussianQuadrature)) {}

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLobatto(
       Real relTolerance, Real absTolerance, Size maxEvaluations, bool useConvergenceEstimate) {
       return Integration(GaussLobatto,
                           ext::shared_ptr<Integrator>(
                               new GaussLobattoIntegral(maxEvaluations,
                                                        absTolerance,
                                                        relTolerance,
                                                        useConvergenceEstimate)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussKronrod(Real absTolerance,
                                                   Size maxEvaluations) {
        return Integration(GaussKronrod,
                           ext::shared_ptr<Integrator>(
                               new GaussKronrodAdaptive(absTolerance,
                                                        maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::simpson(Real absTolerance,
                                               Size maxEvaluations) {
        return Integration(Simpson,
                           ext::shared_ptr<Integrator>(
                               new SimpsonIntegral(absTolerance,
                                                   maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::trapezoid(Real absTolerance,
                                        Size maxEvaluations) {
        return Integration(Trapezoid,
                           ext::shared_ptr<Integrator>(
                              new TrapezoidIntegral<Default>(absTolerance,
                                                             maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLaguerre(Size intOrder) {
        QL_REQUIRE(intOrder <= 192, "maximum integraton order (192) exceeded");
        return Integration(GaussLaguerre,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussLaguerreIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLegendre(Size intOrder) {
        return Integration(GaussLegendre,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussLegendreIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussChebyshev(Size intOrder) {
        return Integration(GaussChebyshev,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussChebyshevIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussChebyshev2nd(Size intOrder) {
        return Integration(GaussChebyshev2nd,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussChebyshev2ndIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::discreteSimpson(Size evaluations) {
        return Integration(
            DiscreteSimpson, ext::shared_ptr<Integrator>(
                new DiscreteSimpsonIntegrator(evaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::discreteTrapezoid(Size evaluations) {
        return Integration(
            DiscreteTrapezoid, ext::shared_ptr<Integrator>(
                new DiscreteTrapezoidIntegrator(evaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::expSinh(Real relTolerance) {
        return Integration(
            ExpSinh, ext::shared_ptr<Integrator>(
                new ExpSinhIntegral(relTolerance)));
    }

    Size AnalyticHestonEngine::Integration::numberOfEvaluations() const {
        if (integrator_ != nullptr) {
            return integrator_->numberOfEvaluations();
        } else if (gaussianQuadrature_ != nullptr) {
            return gaussianQuadrature_->order();
        } else {
            QL_FAIL("neither Integrator nor GaussianQuadrature given");
        }
    }

    bool AnalyticHestonEngine::Integration::isAdaptiveIntegration() const {
        return intAlgo_ == GaussLobatto
            || intAlgo_ == GaussKronrod
            || intAlgo_ == Simpson
            || intAlgo_ == Trapezoid
            || intAlgo_ == ExpSinh;
    }

    Real AnalyticHestonEngine::Integration::calculate(
        Real c_inf,
        const ext::function<Real(Real)>& f,
        const ext::function<Real()>& maxBound,
        const Real scaling) const {

        Real retVal;

        switch(intAlgo_) {
          case GaussLaguerre:
            retVal = (*gaussianQuadrature_)(f);
            break;
          case GaussLegendre:
          case GaussChebyshev:
          case GaussChebyshev2nd:
            retVal = (*gaussianQuadrature_)(integrand1(c_inf, f));
            break;
          case ExpSinh:
            retVal = scaling*(*integrator_)(
                    [scaling, f](Real x) -> Real { return f(scaling*x);},
                0.0, boost::math::tools::max_value<Real>());
            break;
          case Simpson:
          case Trapezoid:
          case GaussLobatto:
          case GaussKronrod:
              if (maxBound && maxBound() != Null<Real>())
                  retVal = (*integrator_)(f, 0.0, maxBound());
              else
                  retVal = (*integrator_)(integrand2(c_inf, f), 0.0, 1.0);
              break;
          case DiscreteTrapezoid:
          case DiscreteSimpson:
              if (maxBound && maxBound() != Null<Real>())
                  retVal = (*integrator_)(f, 0.0, maxBound());
              else
                  retVal = (*integrator_)(integrand3(c_inf, f), 0.0, 1.0);
              break;
          default:
            QL_FAIL("unknwon integration algorithm");
        }

        return retVal;
     }

    Real AnalyticHestonEngine::Integration::calculate(
        Real c_inf,
        const ext::function<Real(Real)>& f,
        Real maxBound) const {

        return AnalyticHestonEngine::Integration::calculate(
            c_inf, f, [=](){ return maxBound; });
    }

    Real AnalyticHestonEngine::Integration::andersenPiterbargIntegrationLimit(
        Real c_inf, Real epsilon, Real v0, Real t) {

        const Real uMaxGuess = -std::log(epsilon)/c_inf;
        const Real uMaxStep = 0.1*uMaxGuess;

        const Real uMax = Brent().solve(u_Max(c_inf, epsilon),
            QL_EPSILON*uMaxGuess, uMaxGuess, uMaxStep);

        try {
            const Real uHatMaxGuess = std::sqrt(-std::log(epsilon)/(0.5*v0*t));
            const Real uHatMax = Brent().solve(uHat_Max(0.5*v0*t, epsilon),
                QL_EPSILON*uHatMaxGuess, uHatMaxGuess, 0.001*uHatMaxGuess);

            return std::max(uMax, uHatMax);
        }
        catch (const Error&) {
            return uMax;
        }
    }
}
