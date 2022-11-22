/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen
 Copyright (C) 2022 Ignacio Anguita

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
       const ext::shared_ptr<HestonModel>& model, Real L, Size N)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
    L_(L), N_(N),
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

        // this is a european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // plain vanilla
        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        const ext::shared_ptr<HestonProcess> process = model_->process();

        const Date maturityDate = arguments_.exercise->lastDate();
        const Time maturity = process->time(maturityDate);

        const Real cum1 = c1(maturity);
        const Real w = std::sqrt(std::fabs(c2(maturity))
            // the 4th order doesn't necessarily improve the precision
            // + std::sqrt(std::fabs(c4(maturity)))
        );

        const Real k = payoff->strike();
        const Real spot = process->s0()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        const DiscountFactor df
            = process->riskFreeRate()->discount(maturityDate);
        const DiscountFactor qf
            = process->dividendYield()->discount(maturityDate);
        const Real fwd = spot*qf/df;
        const Real x = std::log(fwd/k);

        const Real a = x + cum1 - L_*w;
        const Real b = x + cum1 + L_*w;
        
        // Check if it exceeds the truncation bound
        
        if (x >= b/2 || x <= a/2) {
            //returns lower/upper bounds
            if (payoff->optionType() == Option::Put)
                results_.value = std::max(-spot*qf+k*df,0.0);
            else if (payoff->optionType() == Option::Call)
           	    results_.value = std::max(spot*qf-k*df,0.0);
       	    else
                QL_FAIL("unknown payoff type");
            return;
        }
		

        const Real d = 1.0/(b-a);

        const Real expA = std::exp(a);
        Real s = chF(0, maturity).real()*(expA-1-a)*d;

        for (Size n=1; n < N_; ++n) {
            const Real r = n*M_PI*d;
            const Real U_n = 2.0*d*( 1.0/(1.0 + r*r)
                *(expA + r*std::sin(r*a) - std::cos(r*a)) - 1.0/r*std::sin(r*a));

            s += U_n*(chF(r, maturity)
                     *std::exp(std::complex<Real>(0, r*(x-a)))).real();
        }

        if (payoff->optionType() == Option::Put)
            results_.value = k*df*s;
        else if (payoff->optionType() == Option::Call) {
            const DiscountFactor qf
                = process->dividendYield()->discount(maturityDate);
            results_.value = spot*qf - k*df*(1-s);
        }
        else
            QL_FAIL("unknown payoff type");
    }

    Real COSHestonEngine::muT(Time t) const {
        return std::log(  model_->process()->dividendYield()->discount(t)
                        / model_->process()->riskFreeRate()->discount(t));
    }

    std::complex<Real> COSHestonEngine::chF(Real u, Real t)
    const {
        const Real sigma2 = sigma_*sigma_;

        const std::complex<Real> D = std::sqrt(
            squared(std::complex<Real>(kappa_, -rho_*sigma_*u))
            + std::complex<Real>(u*u, u)*sigma2);

        const std::complex<Real> g(kappa_, -rho_*sigma_*u);

        const std::complex<Real> G = (g-D)/(g+D);

        return std::exp(
              v0_/(sigma2)*(1.0-std::exp(-D*t))/(1.0-G*std::exp(-D*t))
             *(g-D) + kappa_*theta_/sigma2*((g-D)*t
                -2.0*std::log((1.0-G*std::exp(-D*t))/(1.0-G)))
            );
   }

   /*
    Mathematica program to calculate the n-th cumulant

    d[z_] := Sqrt[(kappa -i*rho*sigma*z)^2 + (z*z+i*z)*sigma^2]

    g[z_] := (kappa -i*rho*sigma*z - d[z])/(kappa -i*rho*sigma*z + d[z])

    phi[z_] := Exp[ v0/(sigma^2)*(1-Exp[-d[z]*t])/(1-g[z]*Exp[-d[z]*t])
        *(kappa -i*rho*sigma*z - d[z]) + kappa*theta/sigma^2
        *((kappa -i*rho*sigma*z-d[z])*t
          -2*Log[(1-g[z]*Exp[-d[z]*t])/(1-g[z]) ]) ]

    e[z_] := Log[phi[-i*z]]

    // for C++ formatting see
    // http://mathematica.stackexchange.com/questions/114175/cform-not-getting-exp-pow-log-functions
    cpp = RawBoxes[Replace[ToBoxes@#,
        InterpretationBox[a_, b_, c___] :>  With[{aa =
        StringReplace[ a, {"Sqrt" -> "std::sqrt", "Power(E," -> "std::exp(",
                           "Power" -> "std::pow"}]}, aa], {0, Infinity}]] &;

    c[n_] := cpp@CForm[FullSimplify[Derivative[n][e][0],
         kappa > 0 && theta > 0 && v0 > 0 && sigma > 0 &&
         rho [Element] {-1, 1} && i^2 == -1]]
    */

   Real COSHestonEngine::c1(Time t) const {
       return (-theta_ + std::exp(kappa_*t)
           *( theta_ - kappa_*t*theta_ -
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

   Real COSHestonEngine::c4(Time t) const {
       const Real sigma2 = sigma_*sigma_;
       const Real sigma3 = sigma2*sigma_;
       const Real sigma4 = sigma2*sigma2;
       const Real kappa2 = kappa_*kappa_;
       const Real kappa3 = kappa2*kappa_;
       const Real kappa4 = kappa2*kappa2;
       const Real kappa5 = kappa2*kappa3;
       const Real kappa6 = kappa3*kappa3;
       const Real kappa7 = kappa4*kappa3;
       const Real rho2   = rho_*rho_;
       const Real rho3   = rho2*rho_;
       const Time t2     = t*t;
       const Time t3     = t2*t;

       return
           (sigma2*(3*sigma4*(theta_ - 4*v0_) +
           3*std::exp(4*kappa_*t)*((-93*sigma4 +
           64*kappa5*(t + 4*rho2*t) +
           4*kappa_*sigma3*(176*rho_ + 5*sigma_*t) -
           32*kappa2*sigma2*(11 + 50*rho2 +
           5*rho_*sigma_*t) + 32*kappa3*sigma_*(3*sigma_*t + 4*rho_*(10 +
           8*rho2 + 3*rho_*sigma_*t)) - 32*kappa4*(5 +
           4*rho_*(6*rho_ + (3 + 2*rho2)*sigma_*t)))*theta_ +
           4*(4*kappa2 - 4*kappa_*rho_*sigma_ +
           sigma2)*(4*kappa2*(1 + 4*rho2) -
           20*kappa_*rho_*sigma_ + 5*sigma2)*v0_) +
           24*std::exp(kappa_*t)*sigma2*(-2*kappa2*(-1 +
           rho_*sigma_*t)*(theta_ - 3*v0_) + sigma2*(theta_ - 2*v0_) +
           kappa_*sigma_*(-4*rho_*theta_ + sigma_*t*theta_ + 10*rho_*v0_ -
           3*sigma_*t*v0_)) + 12*std::exp(2*kappa_*t)*(sigma4*(7*theta_ -
           4*v0_) + 8*kappa4*(1 + 2*rho_*sigma_*t*(-2 +
           rho_*sigma_*t))*(theta_ - 2*v0_) +
           2*kappa_*sigma3*(-24*rho_*theta_ + 5*sigma_*t*theta_ +
           20*rho_*v0_ - 6*sigma_*t*v0_) + 4*kappa2*sigma2*((6
           + 20*rho2 - 14*rho_*sigma_*t +
           sigma2*t2)*theta_ - 2*(3 + 12*rho2 -
           10*rho_*sigma_*t + sigma2*t2)*v0_) +
           8*kappa3*sigma_*((3*sigma_*t + 2*rho_*(-4 + sigma_*t*(4*rho_ -
           sigma_*t)))*theta_ + 2*(-3*sigma_*t + 2*rho_*(3 + sigma_*t*(-3*rho_ +
           sigma_*t)))*v0_)) -
           8*std::exp(3*kappa_*t)*(16*kappa6*rho2*t2*(-3 + rho_*sigma_*t)*(theta_ - v0_) - 3*sigma4*(7*theta_ +
           2*v0_) + 2*kappa3*sigma_*((192*(rho_ + rho3) -
           6*(9 + 40*rho2)*sigma_*t + 42*rho_*sigma2*t2 -
           sigma3*t3)*theta_ + (-48*rho3 + 18*(1
           + 4*rho2)*sigma_*t - 24*rho_*sigma2*t2
           + sigma3*t3)*v0_) + 12*kappa4*((-4 -
           24*rho2 + 8*rho_*(4 + 3*rho2)*sigma_*t - (3 +
           14*rho2)*sigma2*t2 + rho_*sigma3*t3)*theta_ + (8*rho2 -
           8*rho_*(2 + rho2)*sigma_*t + (3 +
           8*rho2)*sigma2*t2 - rho_*sigma3*t3)*v0_) -
           6*kappa2*sigma2*((15 + 80*rho2 -
           35*rho_*sigma_*t + 2*sigma2*t2)*theta_ + (3 +
           sigma_*t*(7*rho_ - sigma_*t))*v0_) + 24*kappa5*t*((-2 +
           rho_*(4*sigma_*t + rho_*(-8 + sigma_*t*(4*rho_ - sigma_*t))))*theta_ + (2 +
           rho_*(-4*sigma_*t + rho_*(4 + sigma_*t*(-2*rho_ + sigma_*t))))*v0_) +
           3*kappa_*sigma3*(sigma_*t*(-9*theta_ + v0_) + 10*rho_*(6*theta_
           + v0_)))))/(64.*std::exp(4*kappa_*t)*kappa7);
   }

   Real COSHestonEngine::mu(Time t) const {
       return c1(t);
   }
   Real COSHestonEngine::var(Time t) const {
       return c2(t);
   }
   Real COSHestonEngine::skew(Time t) const {
       return c3(t)/std::pow(c2(t), 1.5);
   }
   Real COSHestonEngine::kurtosis(Time t) const {
       return c4(t)/squared(c2(t));
   }

}

