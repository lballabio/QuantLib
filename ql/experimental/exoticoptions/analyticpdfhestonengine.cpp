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
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>

#include <boost/bind.hpp>
#include <cmath>

namespace QuantLib {

    namespace {
        struct Hestonp {
            Real v0, kappa, theta, sigma, rho;
        };

        std::complex<Real> gamma(const Hestonp& p, Real p_x) {
            return std::complex<Real>(p.kappa, p.rho*p.sigma*p_x);
        }

        std::complex<Real> omega(const Hestonp& p, Real p_x) {
           const std::complex<Real> g = gamma(p, p_x);
           return std::sqrt(g*g
                  + p.sigma*p.sigma*std::complex<Real>(p_x*p_x, -p_x));
        }

        std::complex<Real> cpx_pv(const Hestonp& p, Real p_x, Real x, Time t){
            const Real sigma2 = p.sigma*p.sigma;
            const std::complex<Real> g = gamma(p, p_x);
            const std::complex<Real> o = omega(p, p_x);

            return
                std::exp(std::complex<Real>(0.0, p_x*x)
                         - p.v0*std::complex<Real>(p_x*p_x, -p_x)
                           /(g+o*std::cosh(0.5*o*t)/std::sinh(0.5*o*t))
                         + p.kappa*g*p.theta*t/sigma2)
                /std::pow(std::cosh(0.5*o*t)+g/o*std::sinh(0.5*o*t),
                          2.0*p.kappa*p.theta/sigma2);
        }

        Real zero_pv(const Hestonp& p, Real p_x, Real x, Time t){
            return std::abs(cpx_pv(p, p_x, x, t))-QL_EPSILON;
        }

        Real pv(const Hestonp& p, Real p_x, Real x, Time t){
            return cpx_pv(p, p_x, x, t).real();
        }
    }

    AnalyticPDFHestonEngine::AnalyticPDFHestonEngine(
        const boost::shared_ptr<HestonModel>& model,
        Real eps, Size nIterations, Real xMax)
    : eps_(eps), xMax_(xMax),
      nIterations_(nIterations),
      model_(model) {
    }

    void AnalyticPDFHestonEngine::calculate() const {
        // this is an European option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        const boost::shared_ptr<HestonProcess>& process = model_->process();

        const Time t = process->time(arguments_.exercise->lastDate());

        const Real xMax = 10 * std::sqrt(process->theta()*t);

        results_.value = GaussLobattoIntegral(nIterations_, eps_)
            (boost::bind(&AnalyticPDFHestonEngine::weightedPayoff, this,_1, t),
             -xMax, xMax);
    }

    Real AnalyticPDFHestonEngine::Pv(Real s_0, Real s_t, Time t) const {
        const boost::shared_ptr<HestonProcess>& process = model_->process();
        const DiscountFactor d=  process->riskFreeRate()->discount(t)
                               / process->dividendYield()->discount(t);
        const Real x_t = std::log(d * s_t/s_0);

        return Pv(x_t, t);
    }


    Real AnalyticPDFHestonEngine::Pv(Real x_t, Time t) const {
        const Hestonp p = { model_->v0(),
                            model_->kappa(),
                            model_->theta(),
                            model_->sigma(),
                            model_->rho() };
        Real xMax = (xMax_ != Null<Real>()) ? xMax_
            : Brent().solve(boost::bind(&zero_pv, p, _1, x_t, t),
                            0.01, 1.0, 1.0);

        return GaussLobattoIntegral(nIterations_, 0.1*eps_)
               (boost::bind(&pv, p, _1, x_t, t), -xMax, xMax)/M_TWOPI;
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

