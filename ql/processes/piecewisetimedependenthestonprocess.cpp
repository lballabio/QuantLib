/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
#include <ql/math/functional.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/math/modifiedbessel.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <ql/processes/piecewisetimedependenthestonprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>
#include <complex>
#include <utility>

namespace QuantLib {

    PiecewiseTimeDependentHestonProcess::PiecewiseTimeDependentHestonProcess(
                        Handle<YieldTermStructure> riskFreeRate,
                        Handle<YieldTermStructure> dividendYield,
                        Handle<Quote> s0,
                        Real v0,
                        const Parameter& kappa,
                        const Parameter& theta,
                        const Parameter& sigma,
                        const Parameter& rho,
                        TimeGrid timeGrid,
                        Discretization d)
    : StochasticProcess(ext::shared_ptr<discretization>(new EulerDiscretization)),
      riskFreeRate_(std::move(riskFreeRate)), dividendYield_(std::move(dividendYield)),
      s0_(std::move(s0)), v0_(v0), kappa_(kappa), theta_(theta), sigma_(sigma), rho_(rho),
      discretization_(d) {

        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(s0_);
    }

    Size PiecewiseTimeDependentHestonProcess::size() const {
        return 2;
    }

    Size PiecewiseTimeDependentHestonProcess::factors() const {
        return 2;
    }

    Disposable<Array> PiecewiseTimeDependentHestonProcess::initialValues() const {
        Array tmp(2);
        tmp[0] = s0_->value();
        tmp[1] = v0_;
        return tmp;
    }

    Disposable<Array> PiecewiseTimeDependentHestonProcess::drift(Time t, const Array& x) const {
        Array tmp(2);
        const Real vol = (x[1] > 0.0) ? std::sqrt(x[1])
                         : (discretization_ == Reflection) ? - std::sqrt(-x[1])
                         : 0.0;

        tmp[0] = riskFreeRate_->forwardRate(t, t, Continuous)
               - dividendYield_->forwardRate(t, t, Continuous)
               - 0.5 * vol * vol;

        tmp[1] = kappa_(t)*
           (theta_(t)-((discretization_==PartialTruncation) ? x[1] : vol*vol));
        return tmp;
    }

    Disposable<Matrix> PiecewiseTimeDependentHestonProcess::diffusion(Time t, const Array& x) const {
        /* the instantaneous correlation matrix is
           |  1     rho(t) |
           | rho(t)     1  |
           whose square root (which is used here) is
           |  1                0       |
           | rho(t)   sqrt(1-rho(t)^2) |
        */
        Matrix tmp(2,2);
        const Real vol = (x[1] > 0.0) ? std::sqrt(x[1])
                         : (discretization_ == Reflection) ? -std::sqrt(-x[1])
                         : 1e-8; // set vol to (almost) zero but still
                                 // expose some correlation information
        const Real sigma2 = sigma_(t) * vol;
        const Real sqrhov = std::sqrt(1.0 - rho_(t)*rho_(t));

        tmp[0][0] = vol;             tmp[0][1] = 0.0;
        tmp[1][0] = rho_(t)*sigma2;  tmp[1][1] = sqrhov*sigma2;
        return tmp;
    }

    Disposable<Array> PiecewiseTimeDependentHestonProcess::apply(
                                              const Array& x0,
                                              const Array& dx) const {
        Array tmp(2);
        tmp[0] = x0[0] * std::exp(dx[0]);
        tmp[1] = x0[1] + dx[1];
        return tmp;
    }

    Disposable<Array> PiecewiseTimeDependentHestonProcess::evolve(
                            Time t0,
                            const Array& x0,
                            Time dt,
                            const Array& dw) const {
        Array retVal(2);
        Real vol, vol2, mu, nu;

        const Real sdt = std::sqrt(dt);
        const Real sqrhov = std::sqrt(1.0 - rho_(t0)*rho_(t0));

        switch (discretization_) {
          // For the definition of PartialTruncation, FullTruncation
          // and Reflection  see Lord, R., R. Koekkoek and D. van Dijk (2006),
          // "A Comparison of biased simulation schemes for
          //  stochastic volatility models",
          // Working Paper, Tinbergen Institute
          case PartialTruncation:
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            vol2 = sigma_(t0) * vol;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol * vol;
            nu = kappa_(t0)*(theta_(t0) - x0[1]);

            retVal[0] = x0[0] * std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = x0[1] + nu*dt + vol2*sdt*(rho_(t0)*dw[0] + sqrhov*dw[1]);
            break;
          case FullTruncation:
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            vol2 = sigma_(t0) * vol;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol * vol;
            nu = kappa_(t0)*(theta_(t0) - vol*vol);

            retVal[0] = x0[0] * std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = x0[1] + nu*dt + vol2*sdt*(rho_(t0)*dw[0] + sqrhov*dw[1]);
            break;
          case Reflection:
            vol = std::sqrt(std::fabs(x0[1]));
            vol2 = sigma_(t0) * vol;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol*vol;
            nu = kappa_(t0)*(theta_(t0) - vol*vol);

            retVal[0] = x0[0]*std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = vol*vol
                        +nu*dt + vol2*sdt*(rho_(t0)*dw[0] + sqrhov*dw[1]);
            break;
          default:
            QL_FAIL("unknown discretization schema");
        }

        return retVal;
    }

    const Handle<Quote>& PiecewiseTimeDependentHestonProcess::s0() const {
        return s0_;
    }

    const Handle<YieldTermStructure>& PiecewiseTimeDependentHestonProcess::dividendYield() const {
        return dividendYield_;
    }

    const Handle<YieldTermStructure>& PiecewiseTimeDependentHestonProcess::riskFreeRate() const {
        return riskFreeRate_;
    }

    Time PiecewiseTimeDependentHestonProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

    const TimeGrid& PiecewiseTimeDependentHestonProcess::timeGrid() const {
        return timeGrid_;
    }
}
