/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Yee Man Chan

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

#include <ql/math/distributions/chisquaredistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <ql/processes/gjrgarchprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <utility>

namespace QuantLib {

    GJRGARCHProcess::GJRGARCHProcess(Handle<YieldTermStructure> riskFreeRate,
                                     Handle<YieldTermStructure> dividendYield,
                                     Handle<Quote> s0,
                                     Real v0,
                                     Real omega,
                                     Real alpha,
                                     Real beta,
                                     Real gamma,
                                     Real lambda,
                                     Real daysPerYear,
                                     Discretization d)
    : StochasticProcess(ext::shared_ptr<discretization>(new EulerDiscretization)),
      riskFreeRate_(std::move(riskFreeRate)), dividendYield_(std::move(dividendYield)),
      s0_(std::move(s0)), v0_(v0), omega_(omega), alpha_(alpha), beta_(beta), gamma_(gamma),
      lambda_(lambda), daysPerYear_(daysPerYear), discretization_(d) {
        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(s0_);
    }

    Size GJRGARCHProcess::size() const {
        return 2;
    }

    Array GJRGARCHProcess::initialValues() const {
        return { s0_->value(), daysPerYear_*v0_ };
    }

    Array GJRGARCHProcess::drift(Time t, const Array& x) const {
        const Real N = CumulativeNormalDistribution()(lambda_);
        const Real n = std::exp(-lambda_*lambda_/2.0)/std::sqrt(2*M_PI);
        const Real q2 = 1.0 + lambda_*lambda_;
        const Real q3 = lambda_*n + N + lambda_*lambda_*N;
        const Real vol = (x[1] > 0.0) ? std::sqrt(x[1])
                         : (discretization_ == Reflection) ? - std::sqrt(-x[1])
                         : 0.0;

        return {
            riskFreeRate_->forwardRate(t, t, Continuous)
               - dividendYield_->forwardRate(t, t, Continuous)
               - 0.5 * vol * vol,
            daysPerYear_*daysPerYear_*omega_ + daysPerYear_*(beta_ 
                                             + alpha_*q2 + gamma_*q3 - 1.0) *
               ((discretization_==PartialTruncation) ? x[1] : vol*vol)
        };
    }

    Matrix GJRGARCHProcess::diffusion(Time, const Array& x) const {
        /* the correlation matrix is
           |  1   rho |
           | rho   1  |
           whose square root (which is used here) is
           |  1          0       |
           | rho   std::sqrt(1-rho^2) |
        */
        Matrix tmp(2,2);
        const Real N = CumulativeNormalDistribution()(lambda_);
        const Real n = std::exp(-lambda_*lambda_/2.0)/std::sqrt(2*M_PI);
        const Real sigma2 = 2.0 + 4.0*lambda_*lambda_;
        const Real q3 = lambda_*n + N + lambda_*lambda_*N;
        const Real Eml_e4 = lambda_*lambda_*lambda_*n + 5.0*lambda_*n 
            + 3.0*N + lambda_*lambda_*lambda_*lambda_*N 
            + 6.0*lambda_*lambda_*N;
        const Real sigma3 = Eml_e4 - q3*q3;
        const Real sigma12 = -2.0*lambda_;
        const Real sigma13 = -2.0*n - 2*lambda_*N;
        const Real sigma23 = 2.0*N + sigma12*sigma13;
        const Real vol = (x[1] > 0.0) ? std::sqrt(x[1])
                         : (discretization_ == Reflection) ? - std::sqrt(-x[1])
                         : 1e-8; // set vol to (almost) zero but still
                                 // expose some correlation information
        const Real rho1 = std::sqrt(daysPerYear_)*(alpha_*sigma12 
                                            + gamma_*sigma13) * vol * vol;
        const Real rho2 = vol*vol*std::sqrt(daysPerYear_)
            *std::sqrt(alpha_*alpha_*(sigma2 - sigma12*sigma12) 
                       + gamma_*gamma_*(sigma3 - sigma13*sigma13) 
                       + 2.0*alpha_*gamma_*(sigma23 - sigma12*sigma13)); 

            // tmp[0][0], tmp[0][1] are the coefficients of dW_1 and dW_2 
            // in asset return stochastic process
        tmp[0][0] = vol;  tmp[0][1] = 0.0;
        tmp[1][0] = rho1; tmp[1][1] = rho2;
        return tmp;
    }

    Array GJRGARCHProcess::apply(const Array& x0,
                                 const Array& dx) const {
        return { x0[0] * std::exp(dx[0]), x0[1] + dx[1] };
    }

    Array GJRGARCHProcess::evolve(Time t0, const Array& x0,
                                  Time dt, const Array& dw) const {
        Array retVal(2);
        Real vol, mu, nu;

        const Real sdt = std::sqrt(dt);
        const Real N = CumulativeNormalDistribution()(lambda_);
        const Real n = std::exp(-lambda_*lambda_/2.0)/std::sqrt(2*M_PI);
        const Real sigma2 = 2.0 + 4.0*lambda_*lambda_;
        const Real q2 = 1.0 + lambda_*lambda_;
        const Real q3 = lambda_*n + N + lambda_*lambda_*N;
        const Real Eml_e4 = lambda_*lambda_*lambda_*n + 5.0*lambda_*n 
            + 3.0*N + lambda_*lambda_*lambda_*lambda_*N 
            + 6.0*lambda_*lambda_*N;
        const Real sigma3 = Eml_e4 - q3*q3;
        const Real sigma12 = -2.0*lambda_;
        const Real sigma13 = -2.0*n - 2*lambda_*N;
        const Real sigma23 = 2.0*N + sigma12*sigma13;
        const Real rho1 = std::sqrt(daysPerYear_)*(alpha_*sigma12 + gamma_*sigma13);
        const Real rho2 = std::sqrt(daysPerYear_)
            *std::sqrt(alpha_*alpha_*(sigma2 - sigma12*sigma12) 
                       + gamma_*gamma_*(sigma3 - sigma13*sigma13) 
                       + 2.0*alpha_*gamma_*(sigma23 - sigma12*sigma13));

        switch (discretization_) {
          // For the definition of PartialTruncation, FullTruncation
          // and Reflection  see Lord, R., R. Koekkoek and D. van Dijk (2006),
          // "A Comparison of biased simulation schemes for
          //  stochastic volatility models",
          // Working Paper, Tinbergen Institute
          case PartialTruncation:
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol * vol;
            nu = daysPerYear_*daysPerYear_*omega_ 
                + daysPerYear_*(beta_ + alpha_*q2 + gamma_*q3 - 1.0) * x0[1];

            retVal[0] = x0[0] * std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = x0[1] + nu*dt + sdt*vol*vol*(rho1*dw[0] + rho2*dw[1]);
            break;
          case FullTruncation:
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol * vol;
            nu = daysPerYear_*daysPerYear_*omega_ 
                + daysPerYear_*(beta_ + alpha_*q2 + gamma_*q3 - 1.0) * vol *vol;

            retVal[0] = x0[0] * std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = x0[1] + nu*dt + sdt*vol*vol*(rho1*dw[0] + rho2*dw[1]);
            break;
          case Reflection:
            vol = std::sqrt(std::fabs(x0[1]));
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol*vol;
            nu = daysPerYear_*daysPerYear_*omega_ 
                + daysPerYear_*(beta_ + alpha_*q2 + gamma_*q3 - 1.0) * vol * vol;

            retVal[0] = x0[0]*std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = vol*vol
                        +nu*dt + sdt*vol*vol*(rho1*dw[0] + rho2*dw[1]);
            break;
          default:
            QL_FAIL("unknown discretization schema");
        }

        return retVal;
    }

    const Handle<Quote>& GJRGARCHProcess::s0() const {
        return s0_;
    }

    const Handle<YieldTermStructure>& GJRGARCHProcess::dividendYield() const {
        return dividendYield_;
    }

    const Handle<YieldTermStructure>& GJRGARCHProcess::riskFreeRate() const {
        return riskFreeRate_;
    }

    Time GJRGARCHProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

}
