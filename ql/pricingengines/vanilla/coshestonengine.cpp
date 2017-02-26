/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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
#include <ql/pricingengines/vanilla/coshestonengine.hpp>

namespace QuantLib {

    COSHestonEngine::COSHestonEngine(
       const boost::shared_ptr<HestonModel>& model)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
    kappa_(model_->kappa()),
    theta_(model_->theta()),
    sigma_(model_->sigma()),
    rho_  (model_->rho())  ,
    v0_   (model_->v0()) { }


    void COSHestonEngine::update() {
        kappa_ = model_->kappa();
        theta_ = model_->theta();
        sigma_ = model_->sigma();
        rho_   = model_->rho();
        v0_    = model_->v0();

        GenericModelEngine<HestonModel,
                           VanillaOption::arguments,
                           VanillaOption::results>::update();
    }


    void COSHestonEngine::calculate() const {

    }

    Real COSHestonEngine::muT(Time t) const {
        return std::log(  model_->process()->dividendYield()->discount(t)
                        / model_->process()->riskFreeRate()->discount(t));
    }

    std::complex<Real> COSHestonEngine::characteristicFct(Real u, Real t)
    const {
        const Real sigma2 = sigma_*sigma_;

        const std::complex<Real> D = std::sqrt(
            square<std::complex<Real> >()(
                std::complex<Real>(kappa_, -rho_*sigma_*u))
            + std::complex<Real>(u*u, u)*sigma2);

        const std::complex<Real> g(kappa_, -rho_*sigma_*u);

        const std::complex<Real> G = (g-D)/(g+D);

        return std::exp(std::complex<Real>(0.0, u*muT(t))
            + v0_/(sigma2)*(1.0-std::exp(-D*t))/(1.0-G*std::exp(-D*t))
             *(g-D) + kappa_*theta_/sigma2*((g-D)*t
                -2.0*std::log((1.0-G*std::exp(-D*t))/(1.0-G)))
            );
   }

   /*
    Mathematica program to calculate the n-th cumulant

    d[z_] := Sqrt[(kappa_ -i*rho_*sigma_*z)^2 + (z*z+i*z)*sigma_*sigma_]

    g[z_] := (kappa_ -i*rho_*sigma_*z - d[z])/(kappa_ -i*rho_*sigma_*z + d[z])

    phi[z_] := Exp[ i*z*muT + v0_/(sigma_^2)*(1-Exp[-d[z]*t])/(1-g[z]*Exp[-d[z]*t])
        *(kappa_ -i*rho_*sigma_*z - d[z]) + kappa_*theta_/sigma_^2
        *((kappa_ -i*rho_*sigma_*z-d[z])*t
          -2*Log[(1-g[z]*Exp[-d[z]*t])/(1-g[z]) ]) ]

    e[z_] := Log[phi[-i*z]]

    // for C++ formatting see
    // http://mathematica.stackexchange.com/questions/114175/cform-not-getting-exp-pow-log-functions
    cpp = RawBoxes[Replace[ToBoxes@#,
        InterpretationBox[a_, b_, c___] :>  With[{aa =
        StringReplace[ a, {"Sqrt" -> "std::sqrt", "Power(E," -> "std::exp(",
                           "Power" -> "std::pow"}]}, aa], {0, Infinity}]] &;

    c[n_] := cpp@CForm[FullSimplify[Derivative[n][e][0],
         kappa_ > 0 && theta_ > 0 && v0_ > 0 && sigma_ > 0 &&
         rho_ [Element] {-1, 1} && i^2 == -1]]
    */

   Real COSHestonEngine::c1(Time t) const {
       return (-theta_ + std::exp(kappa_*t)
           *(2*kappa_*muT(t) + theta_ - kappa_*t*theta_ -
               v0_) + v0_)/(2*std::exp(kappa_*t)*kappa_);
   }

   Real COSHestonEngine::c2(Time t) const {
       const Real sigma2 = sigma_*sigma_;
       const Real kappa2 = kappa_*kappa_;
       const Real kappa3 = kappa2*kappa_;

       return (sigma2*(theta_ - 2*v0_) +
           std::exp(2*kappa_*t)*(8*kappa3*t*theta_ -
           8*kappa2*(theta_ + rho_*sigma_*t*theta_ - v0_) +
           sigma2*(-5*theta_ + 2*v0_) + 2*kappa_*sigma_*(8*rho_*theta_ +
           sigma_*t*theta_ - 4*rho_*v0_)) +
           4*std::exp(kappa_*t)*(sigma2*theta_ -
           2*kappa2*(-1 + rho_*sigma_*t)*(theta_ - v0_) +
           kappa_*sigma_*(sigma_*t*(theta_ - v0_) + 2*rho_*(-2*theta_ +
           v0_))))/(8.*std::exp(2*kappa_*t)*kappa3);
   }

   Real COSHestonEngine::c3(Time t) const {
       const Real sigma2 = sigma_*sigma_;
       const Real sigma3 = sigma2*sigma_;
       const Real kappa2 = kappa_*kappa_;
       const Real kappa3 = kappa2*kappa_;
       const Real kappa4 = kappa3*kappa_;
       const Real rho2   = rho_*rho_;

       return
         -(sigma_*(sigma3*(theta_ - 3*v0_) +
           std::exp(3*kappa_*t)*(2*(-11*sigma3 -
           24*kappa4*rho_*t + 3*kappa_*sigma2*(20*rho_ +
           sigma_*t) - 6*kappa2*sigma_*(5 + 3*rho_*(4*rho_ + sigma_*t)) +
           12*kappa3*(sigma_*t + 2*rho_*(2 + rho_*sigma_*t)))*theta_ -
           6*(2*kappa_*rho_ - sigma_)*(4*kappa2 - 4*kappa_*rho_*sigma_ +
           sigma2)*v0_) + 6*std::exp(kappa_*t)*sigma_*(-2*kappa2*(-1 +
           rho_*sigma_*t)*(theta_ - 2*v0_) + sigma2*(theta_ - v0_) +
           kappa_*sigma_*(-4*rho_*theta_ + sigma_*t*theta_ + 6*rho_*v0_ - 2*sigma_*t*v0_)) +
           3*std::exp(2*kappa_*t)*(2*kappa_*sigma2*(-16*rho_*theta_ +
           sigma_*t*(3*theta_ - v0_)) + 8*kappa4*rho_*t*(-2 +
           rho_*sigma_*t)*(theta_ - v0_) + sigma3*(5*theta_ + v0_) +
           8*kappa3*(-(rho_*(4 + sigma2*t*t)*theta_) + 2*sigma_*t*(theta_ - v0_) +
           2*rho2*sigma_*t*(2*theta_ - v0_) + rho_*(2 +
           sigma2*t*t)*v0_) + 2*kappa2*sigma_*((8
           + 24*rho2 - 16*rho_*sigma_*t + sigma2*t*t)*theta_ - (8*rho2 -
           8*rho_*sigma_*t + sigma2*t*t)*v0_))))/(16.*std::exp(3*kappa_*t)*
           kappa_*kappa4);
   }
}



