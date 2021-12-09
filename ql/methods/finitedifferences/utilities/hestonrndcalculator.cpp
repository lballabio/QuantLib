/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

#include <ql/math/functional.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/methods/finitedifferences/utilities/bsmrndcalculator.hpp>
#include <ql/methods/finitedifferences/utilities/hestonrndcalculator.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <complex>
#include <utility>

namespace QuantLib {

namespace {
        struct HestonParams {
            Real v0, kappa, theta, sigma, rho;
        };

        HestonParams getHestonParams(
            const ext::shared_ptr<HestonProcess>& process) {
            const HestonParams p = { process->v0(),    process->kappa(),
                                     process->theta(), process->sigma(),
                                     process->rho() };
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

        class CpxPv_Helper {
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

            const HestonParams p_;
            const Time t_;
            const Real x_, c_inf_;
        };
    }


    HestonRNDCalculator::HestonRNDCalculator(ext::shared_ptr<HestonProcess> hestonProcess,
                                             Real integrationEps,
                                             Size maxIntegrationIterations)
    : hestonProcess_(std::move(hestonProcess)), x0_(std::log(hestonProcess_->s0()->value())),
      integrationEps_(integrationEps), maxIntegrationIterations_(maxIntegrationIterations) {}

    Real HestonRNDCalculator::x_t(Real x, Time t) const {
        const DiscountFactor dr = hestonProcess_->riskFreeRate()->discount(t);
        const DiscountFactor dq = hestonProcess_->dividendYield()->discount(t);

        return x - x0_ + std::log(dr/dq);
    }

    Real HestonRNDCalculator::pdf(Real x, Time t) const {
        return GaussLobattoIntegral(
            maxIntegrationIterations_, 0.1*integrationEps_)(
            CpxPv_Helper(getHestonParams(hestonProcess_), x_t(x, t), t),
            0.0, 1.0)/M_TWOPI;
    }

    Real HestonRNDCalculator::cdf(Real x, Time t) const {
        CpxPv_Helper helper(getHestonParams(hestonProcess_), x_t(x, t), t);

        return GaussLobattoIntegral(maxIntegrationIterations_, 0.1*integrationEps_)(
            [&](Real p_x){ return helper.p0(p_x); },
            0.0, 1.0)/M_TWOPI + 0.5;
    }

    Real HestonRNDCalculator::invcdf(Real p, Time t) const {
        const Real v0    = hestonProcess_->v0();
        const Real kappa = hestonProcess_->kappa();
        const Real theta = hestonProcess_->theta();

        const Volatility expVol
            = std::sqrt(theta + (v0-theta)*(1-std::exp(-kappa*t))/(t*kappa));

        const ext::shared_ptr<BlackScholesMertonProcess> bsmProcess(
            ext::make_shared<BlackScholesMertonProcess>(
                hestonProcess_->s0(),
                hestonProcess_->dividendYield(),
                hestonProcess_->riskFreeRate(),
                Handle<BlackVolTermStructure>(
                    ext::make_shared<BlackConstantVol>(
                            hestonProcess_->riskFreeRate()->referenceDate(),
                            NullCalendar(),
                            expVol,
                            hestonProcess_->riskFreeRate()->dayCounter()))));

        const Real guess = BSMRNDCalculator(bsmProcess).invcdf(p, t);

        return RiskNeutralDensityCalculator::InvCDFHelper(
            this, guess, 0.1*integrationEps_, maxIntegrationIterations_)
            .inverseCDF(p, t);
    }
}
