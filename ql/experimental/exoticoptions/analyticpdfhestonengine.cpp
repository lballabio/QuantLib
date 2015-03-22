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

/*! \file analyticpdfhestonengine.cpp
    \brief Analytic engine for arbitrary European payoffs under the Heston model
*/

#include <ql/math/functional.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/bind.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

#include <cmath>
#include <complex>

namespace QuantLib {

    namespace {
        struct HestonParams {
            Real v0, kappa, theta, sigma, rho;
        };

        HestonParams getHestonParams(
            const boost::shared_ptr<HestonModel>& model) {
            const HestonParams p = { model->v0(),    model->kappa(),
                                     model->theta(), model->sigma(),
                                     model->rho() };
            return p;
        }

        std::complex<Real> gamma(const HestonParams& p, Real p_x) {
            return std::complex<Real>(p.kappa, p.rho*p.sigma*p_x);
        }

        std::complex<Real> omega(const HestonParams& p, Real p_x) {
           const std::complex<Real> g = gamma(p, p_x);
           return std::sqrt(g*g
                  + p.sigma*p.sigma*std::complex<Real>(p_x*p_x, -p_x));
        }

        class CpxPv_Helper
            : public std::unary_function<Real, Real > {
          public:
            CpxPv_Helper(const HestonParams& p, Real x, Time t)
              : p_(p), t_(t), x_(x),
                c_inf_(std::min(10.0, std::max(0.0001,
                      std::sqrt(1.0-square<Real>()(p_.rho))/p_.sigma))
                      *(p_.v0 + p_.kappa*p_.theta*t))  {}

            Real operator()(Real x) const {
                return std::real(transformPhi(x));
            }

            Real p0(Real p_x) const {
                if (p_x < QL_EPSILON) {
                    return 0.0;
                }

                const Real u_x = std::max(QL_EPSILON, -std::log(p_x)/c_inf_);
                return std::real(phi(u_x)
                        *std::exp(std::complex<Real>(0.0, -2*u_x*x_))
                        /((p_x*c_inf_)*std::complex<Real>(0.0, u_x)));
            }

          private:
            std::complex<Real> transformPhi(Real x) const {
                if (x < QL_EPSILON) {
                    return std::complex<Real>(0.0, 0.0);
                }

                const Real u_x = -std::log(x)/c_inf_;
                return phi(u_x)/(x*c_inf_);
            }

            std::complex<Real> phi(Real p_x) const {
                const Real sigma2 = p_.sigma*p_.sigma;
                const std::complex<Real> g = gamma(p_, p_x);
                const std::complex<Real> o = omega(p_, p_x);
                const std::complex<Real> gamma = (g-o)/(g+o);

                return 2.0*std::exp(std::complex<Real>(0.0, p_x*x_)
                        - p_.v0*std::complex<Real>(p_x*p_x, -p_x)
                          /(g+o*(1.0+std::exp(-o*t_))/(1.0-std::exp(-o*t_)))
                         +p_.kappa*p_.theta/sigma2*(
                           (g-o)*t_ - 2.0*std::log((1.0-gamma*std::exp(-o*t_))
                                                               /(1.0-gamma))));
            }

            const HestonParams& p_;
            const Time t_;
            const Real x_, c_inf_;
        };
    }

    AnalyticPDFHestonEngine::AnalyticPDFHestonEngine(
        const boost::shared_ptr<HestonModel>& model,
        Real gaussLobattoEps,
        Size gaussLobattoIntegrationOrder)
    : gaussLobattoIntegrationOrder_(gaussLobattoIntegrationOrder),
      gaussLobattoEps_(gaussLobattoEps),
      model_(model) {  }

    void AnalyticPDFHestonEngine::calculate() const {
        // this is an European option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        const boost::shared_ptr<HestonProcess>& process = model_->process();

        const Time t = process->time(arguments_.exercise->lastDate());

        const Real xMax = 8.0 * std::sqrt(process->theta()*t
            + (process->v0() - process->theta())
                *(1-std::exp(-process->kappa()*t))/process->kappa());

        results_.value = GaussLobattoIntegral(
            gaussLobattoIntegrationOrder_, gaussLobattoEps_)(
            boost::bind(&AnalyticPDFHestonEngine::weightedPayoff, this,_1, t),
                         -xMax, xMax);
    }

    Real AnalyticPDFHestonEngine::Pv(Real x_t, Time t) const {
        return GaussLobattoIntegral(
            gaussLobattoIntegrationOrder_, 0.1*gaussLobattoEps_)(
                CpxPv_Helper(getHestonParams(model_), x_t, t),
                0.0, 1.0)/M_TWOPI;
    }

    Real AnalyticPDFHestonEngine::cdf(Real s, Time t) const {
        const boost::shared_ptr<HestonProcess>& process = model_->process();
        const DiscountFactor d=  process->riskFreeRate()->discount(t)
                               / process->dividendYield()->discount(t);

        const Real s_t = process->s0()->value()/d;
        const Real x = std::log(s_t/s);

        return GaussLobattoIntegral(
            gaussLobattoIntegrationOrder_, gaussLobattoEps_)(
                boost::bind(&CpxPv_Helper::p0,
                    CpxPv_Helper(getHestonParams(model_), x, t), _1),
                0.0, 1.0)/M_TWOPI + 0.5;
    }

    Real AnalyticPDFHestonEngine::weightedPayoff(Real x_t, Time t) const {
        const boost::shared_ptr<HestonProcess>& process = model_->process();

        const Real s_0 = process->s0()->value();
        const DiscountFactor rD = process->riskFreeRate()->discount(t);
        const DiscountFactor dD = process->dividendYield()->discount(t);

        const Real s_t = s_0*std::exp(x_t)*dD/rD;
        const Real payoff = (*arguments_.payoff)(s_t);

        return (payoff != 0.0) ? payoff*Pv(x_t, t)*rD : 0.0;
    }
}

