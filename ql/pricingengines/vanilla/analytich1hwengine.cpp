/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file analytichestonengine.cpp
    \brief analytic Heston-Hull-White engine based on the H1-HW approximation
*/

#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/pricingengines/vanilla/analytich1hwengine.hpp>

namespace QuantLib {
    // integration helper class
    class AnalyticH1HWEngine::Fj_Helper
        : public std::unary_function<std::complex<Real>, Real> {

      public:
        Fj_Helper(const Handle<HestonModel>& hestonModel,
                  const boost::shared_ptr<HullWhite>& hullWhiteModel,
                  Real rho_xr, Time term, Real strike, Size j);

        std::complex<Real> operator()(Real u) const;

      private:
        Real c(Time t) const;
        Real lambda(Time t) const;
        Real Lambda(Time t) const;
        Real LambdaApprox(Time t) const;

        const Size j_;
        const Real lambda_, eta_;
        const Real v0_, kappa_, theta_, gamma_;
        const Real d_;
        const Real rhoSr_;
        const Time term_;
    };

    AnalyticH1HWEngine::Fj_Helper::Fj_Helper(
        const Handle<HestonModel>& hestonModel,
        const boost::shared_ptr<HullWhite>& hullWhiteModel,
        Real rhoSr, Time term, Real, Size j)
    : j_     (j),
      lambda_(hullWhiteModel->a()),
      eta_   (hullWhiteModel->sigma()),
      v0_    (hestonModel->v0()),
      kappa_ (hestonModel->kappa()),
      theta_ (hestonModel->theta()),
      gamma_ (hestonModel->sigma()),
      d_     (4.0*kappa_*theta_/(gamma_*gamma_)),
      rhoSr_ (rhoSr),
      term_  (term) {
    }

    Real AnalyticH1HWEngine::Fj_Helper::c(Time t) const {
        return gamma_*gamma_/(4*kappa_)*(1.0-std::exp(-kappa_*t));
    }

    Real AnalyticH1HWEngine::Fj_Helper::lambda(Time t) const {
        return  4.0*kappa_*v0_*std::exp(-kappa_*t)
               /(gamma_*gamma_*(1.0-std::exp(-kappa_*t)));
    }

    Real AnalyticH1HWEngine::Fj_Helper::LambdaApprox(Time t) const {
        return std::sqrt( c(t)*(lambda(t)-1.0)
                        + c(t)*d_*(1.0 + 1.0/(2.0*(d_+lambda(t)))));
    }

    Real AnalyticH1HWEngine::Fj_Helper::Lambda(Time t) const {
        const GammaFunction g = GammaFunction();
        const Size maxIter = 1000;
        const Real lambdaT = lambda(t);

        Size i=0;
        Real retVal = 0.0, s;

        do {
            Real k = static_cast<Real>(i);
            s=std::exp(k*std::log(0.5*lambdaT) + g.logValue(0.5*(1+d_)+k)
                        - g.logValue(k+1) - g.logValue(0.5*d_+k));
            retVal += s;
        } while (s > std::numeric_limits<float>::epsilon() && ++i < maxIter);

        QL_REQUIRE(i < maxIter, "can not calculate Lambda");

        retVal *= std::sqrt(2*c(t)) * std::exp(-0.5*lambdaT);
        return retVal;
    }

    std::complex<Real> AnalyticH1HWEngine::Fj_Helper::operator()(Real u) const {

        const Real gamma2 = gamma_*gamma_;

        Real a, b, c;
        if (8.0*kappa_*theta_/gamma2 > 1.0) {
            a = std::sqrt(theta_-gamma2/(8.0*kappa_));
            b = std::sqrt(v0_) - a;
            c =-std::log((LambdaApprox(1.0)-a)/b);
        }
        else {
            a = std::sqrt(gamma2/(2.0*kappa_))
                *std::exp(  GammaFunction().logValue(0.5*(d_+1.0))
                          - GammaFunction().logValue(0.5*d_));

            const Time t1 = 0.0;
            const Time t2 = 1.0/kappa_;

            const Real Lambda_t1 = std::sqrt(v0_);
            const Real Lambda_t2 = Lambda(t2);

            c = std::log((Lambda_t2-a)/(Lambda_t1-a))/(t1-t2);
            b = std::exp(c*t1)*(Lambda_t1-a);
        }

        const std::complex<Real> I4 =
            -1.0/lambda_* std::complex<Real>(u*u, ((j_ == 1u)? -u : u))
              *(  b/c*(1.0 - std::exp(-c*term_))
                + a*term_
                + a/lambda_*(std::exp(-lambda_*term_) - 1.0)
                + b/(c-lambda_)*std::exp(-c*term_)
                    *(1.0 - std::exp(-term_*(lambda_-c))) );

        return eta_*rhoSr_*I4;
    }


    AnalyticH1HWEngine::AnalyticH1HWEngine(
        const boost::shared_ptr<HestonModel>& model,
        const boost::shared_ptr<HullWhite>& hullWhiteModel,
        Real rhoSr, Size integrationOrder)
    : AnalyticHestonHullWhiteEngine(model, hullWhiteModel, integrationOrder),
      rhoSr_(rhoSr) {
        QL_REQUIRE(rhoSr_ >= 0.0, "Fourier integration is not stable if "
                    "the equity interest rate correlation is negative");
    }

    AnalyticH1HWEngine::AnalyticH1HWEngine(
        const boost::shared_ptr<HestonModel>& model,
        const boost::shared_ptr<HullWhite>& hullWhiteModel,
        Real rhoSr, Real relTolerance, Size maxEvaluations)
    : AnalyticHestonHullWhiteEngine(model, hullWhiteModel,
                                    relTolerance, maxEvaluations),
      rhoSr_(rhoSr) {
    }

    std::complex<Real> AnalyticH1HWEngine::addOnTerm(Real u, Time t, Size j)
    const {
        return AnalyticHestonHullWhiteEngine::addOnTerm(u, t, j)
               + Fj_Helper(model_, hullWhiteModel_, rhoSr_, t, 0.0, j)(u);
    }
}

