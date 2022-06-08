/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Jack Gillett

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

#include <ql/experimental/asian/analytic_cont_geom_av_price_heston.hpp>
#include <utility>

namespace QuantLib {

    class AnalyticContinuousGeometricAveragePriceAsianHestonEngine::Integrand {
      private:
        Real t_, T_, K_, logK_;
        Size cutoff_;
        const AnalyticContinuousGeometricAveragePriceAsianHestonEngine* const parent_;
        Real xiRightLimit_;
        std::complex<Real> i_;

      public:
        Integrand(Real T,
                  Size cutoff,
                  Real K,
                  const AnalyticContinuousGeometricAveragePriceAsianHestonEngine* const parent,
                  Real xiRightLimit) : t_(0.0), T_(T), K_(K), logK_(std::log(K)), cutoff_(cutoff),
                                       parent_(parent), xiRightLimit_(xiRightLimit), i_(std::complex<Real>(0.0, 1.0)) {}

        Real operator()(Real xi) const {
            Real xiDash = (0.5+1e-8+0.5*xi) * xiRightLimit_; // Map xi to full range

            std::complex<Real> inner1 = parent_->Phi(1.0 + xiDash*i_, 0, T_, t_, cutoff_);
            std::complex<Real> inner2 = - K_*parent_->Phi(xiDash*i_, 0, T_, t_, cutoff_);

            return 0.5*xiRightLimit_*std::real((inner1 + inner2) * std::exp(-xiDash*logK_*i_) / (xiDash*i_));
        }
    };

    class AnalyticContinuousGeometricAveragePriceAsianHestonEngine::DcfIntegrand {
      private:
        Real t_, T_, denominator_;
        const Handle<YieldTermStructure> riskFreeRate_;
        const Handle<YieldTermStructure> dividendYield_;
      public:
        DcfIntegrand(Real t,
                     Real T,
                     Handle<YieldTermStructure> riskFreeRate,
                     Handle<YieldTermStructure> dividendYield)
        : t_(t), T_(T), riskFreeRate_(std::move(riskFreeRate)),
          dividendYield_(std::move(dividendYield)) {
            denominator_ = std::log(riskFreeRate_->discount(t_)) - std::log(dividendYield_->discount(t_));
        }

        Real operator()(Real u) const {
            Real uDash = (0.5+1e-8+0.5*u) * (T_ - t_) + t_; // Map u to full range
            return 0.5*(T_ - t_)*(-std::log(riskFreeRate_->discount(uDash))
                               + std::log(dividendYield_->discount(uDash)) + denominator_);
        }
    };


    AnalyticContinuousGeometricAveragePriceAsianHestonEngine::
        AnalyticContinuousGeometricAveragePriceAsianHestonEngine(
            ext::shared_ptr<HestonProcess> process, Size summationCutoff, Real xiRightLimit)
    : process_(std::move(process)), a3_(0.0), a4_(0.0), a5_(0.0), summationCutoff_(summationCutoff),
      xiRightLimit_(xiRightLimit), integrator_(128) {
        registerWith(process_);

        v0_ = process_->v0();
        rho_ = process_->rho();
        kappa_ = process_->kappa();
        theta_ = process_->theta();
        sigma_ = process_->sigma();
        s0_ = process_->s0();

        riskFreeRate_ = process_->riskFreeRate();
        dividendYield_ = process_->dividendYield();

        // Some of the required constant intermediate variables can be calculated now
        // (although anything depending on T will need to be calculated dynamically later)
        a1_ = 2.0 * v0_ / (sigma_*sigma_);
        a2_ = 2.0 * kappa_ * theta_ / (sigma_*sigma_);
    }

    std::complex<Real> AnalyticContinuousGeometricAveragePriceAsianHestonEngine::z1_f(
            const std::complex<Real>& s, const std::complex<Real>& w, Real T) const {
        return s*s*(1-rho_*rho_)/(2*T*T);
    }

    std::complex<Real> AnalyticContinuousGeometricAveragePriceAsianHestonEngine::z2_f(
            const std::complex<Real>& s, const std::complex<Real>& w, Real T) const {
        return s*(2*rho_*kappa_ - sigma_)/(2*sigma_*T) + s*w*(1-rho_*rho_)/T;
    }

    std::complex<Real> AnalyticContinuousGeometricAveragePriceAsianHestonEngine::z3_f(
            const std::complex<Real>& s, const std::complex<Real>& w, Real T) const {
        return s*rho_/(sigma_*T) + 0.5*w*(2*rho_*kappa_ - sigma_)/sigma_ + 0.5*w*w*(1-rho_*rho_);
    }

    std::complex<Real> AnalyticContinuousGeometricAveragePriceAsianHestonEngine::z4_f(
            const std::complex<Real>& s, const std::complex<Real>& w) const {
        return w*rho_/sigma_;
    }

    std::complex<Real> AnalyticContinuousGeometricAveragePriceAsianHestonEngine::f(const std::complex<Real>& z1,
                                                                                   const std::complex<Real>& z2,
                                                                                   const std::complex<Real>& z3,
                                                                                   const std::complex<Real>& z4,
                                                                                   int n, // Can't use Size here as n can be negative
                                                                                   Real tau) const {;
        std::complex<Real> result;

        // This equation is highly recursive, use dynamic programming with a mutable variable
        // to record the results of previous calls
        if (n<2) {
            if (n<0) {
                result = 0.0;
            } else if (n==0) {
                result = 1.0;
            } else {
                result = 0.5*(kappa_ - z4*sigma_*sigma_)*tau;
            }
        } else {
            std::complex<Real> fMinusN[4];
            Real prefactor = -0.5*sigma_*sigma_*tau*tau / (n*(n-1));

            // For each offset, look up the value in the map and only evaluate function if it's not there
            for (int offset=1; offset<5; offset++) {
                int location = n-offset;
                std::map<int, std::complex<Real> >::const_iterator position = fLookupTable_.find(location);
                if (position != fLookupTable_.end()) {
                    std::complex<Real> value = position->second;
                    fMinusN[offset-1] = value;
                } else {
                    fMinusN[offset-1] = f(z1, z2, z3, z4, location, tau);
                }
            }

            result = prefactor * (z1*tau*tau*fMinusN[3] + z2*tau*fMinusN[2] + (z3 - 0.5*kappa_*kappa_/(sigma_*sigma_))*fMinusN[1]);
        }

        // Store this value in our mutable lookup map
        fLookupTable_[n] = result;

        return result;
    };

    std::pair<std::complex<Real>, std::complex<Real> >
        AnalyticContinuousGeometricAveragePriceAsianHestonEngine::F_F_tilde(
            const std::complex<Real>& z1,
            const std::complex<Real>& z2,
            const std::complex<Real>& z3,
            const std::complex<Real>& z4,
            Real tau,
            Size cutoff) const {
        std::complex<Real> temp = 0.0;
        std::complex<Real> runningSum1 = 0.0;
        std::complex<Real> runningSum2 = 0.0;

        for (Size i=0; i<cutoff; i++) {
            temp = f(z1, z2, z3, z4, i, tau);
            runningSum1 += temp;
            runningSum2 += temp*Real(i)/tau;
        }

        std::pair<std::complex<Real>, std::complex<Real> > result(runningSum1, runningSum2);

        return result;
    };

    std::complex<Real> AnalyticContinuousGeometricAveragePriceAsianHestonEngine::Phi(
            const std::complex<Real>& s,
            const std::complex<Real>& w,
            Real T,
            Real t,
            Size cutoff) const {
        Real tau = T - t;

        std::complex<Real> z1 = z1_f(s, w, T);
        std::complex<Real> z2 = z2_f(s, w, T);
        std::complex<Real> z3 = z3_f(s, w, T);
        std::complex<Real> z4 = z4_f(s, w);

        // Clear the mutable lookup map before calling fLookupTable
        fLookupTable_ = std::map<int, std::complex<Real> >();
        std::pair<std::complex<Real>, std::complex<Real> > temp = F_F_tilde(z1, z2, z3, z4, tau, cutoff);

        std::complex<Real> F, F_tilde;
        F = temp.first;
        F_tilde = temp.second;

        return std::exp(-a1_*F_tilde/F - a2_*std::log(F) + a3_*s + a4_*w + a5_);
    }

    void AnalyticContinuousGeometricAveragePriceAsianHestonEngine::calculate() const {
        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real strike = payoff->strike();
        Date exercise = arguments_.exercise->lastDate();

        Time expiryTime = this->process_->time(exercise);
        QL_REQUIRE(expiryTime >= 0.0, "Expiry Date cannot be in the past");

        Real expiryDcf = riskFreeRate_->discount(expiryTime);
        Real expiryDividendDiscount = dividendYield_->discount(expiryTime);

        // TODO: extend to cover seasoned options (discussed in paper)
        Time startTime = 0.0;

        // These parameters only need to be calculated once per pricing, but are
        // functions of t and T so need to be reset in calculate()
        Time t = startTime;
        Time T = expiryTime;
        Time tau = T - t;
        Real logS0 = std::log(s0_->value());

        // To deal with non-constant rates and dividends, we reformulate Eq.s (14) to (17) with
        // r_ --> (r(t) - q(t)), which gives the new expressions for a3 and a4 used below
        Real dcf = riskFreeRate_->discount(T) / riskFreeRate_->discount(t);
        Real qdcf = dividendYield_->discount(T) / dividendYield_->discount(t);
        DcfIntegrand dcfIntegrand = DcfIntegrand(t, T, riskFreeRate_, dividendYield_);
        Real integratedDcf = integrator_(dcfIntegrand);

        a3_ = (tau*logS0 + integratedDcf)/T - kappa_*theta_*rho_*tau*tau/(2*sigma_*T) - rho_*tau*v0_/(sigma_*T);
        a4_ = logS0*qdcf/dcf - rho_*v0_/sigma_ + rho_*kappa_*theta_*tau/sigma_;
        a5_ = (kappa_*v0_ + kappa_*kappa_*theta_*tau) / (sigma_*sigma_);

        // Calculate the two terms in eq (29) - Phi(1,0) is real (asian forward) but need to type convert
        Real term1 = 0.5 * (std::real(Phi(1,0, T, t, summationCutoff_)) - strike);

        Integrand integrand(T, summationCutoff_, strike, this, xiRightLimit_);
        Real term2 = integrator_(integrand) / M_PI;

        // Apply the payoff functions
        Real value = 0.0;
        switch (payoff->optionType()){
            case Option::Call:
                value = expiryDcf * (term1 + term2);
                break;
            case Option::Put:
                value = expiryDcf * (-term1 + term2);
                break;
            default:
                QL_FAIL("unknown option type");
            }

        results_.value = value;

        results_.additionalResults["dcf"] = expiryDcf;
        results_.additionalResults["qf"] = expiryDividendDiscount;
        results_.additionalResults["s0"] = s0_->value();
        results_.additionalResults["strike"] = strike;
        results_.additionalResults["expiryTime"] = expiryTime;
        results_.additionalResults["exercise"] = exercise;

        results_.additionalResults["term1"] = term1;
        results_.additionalResults["term2"] = term2;
        results_.additionalResults["xiRightLimit"] = xiRightLimit_;
        results_.additionalResults["summationCutoff"] = summationCutoff_;

        results_.additionalResults["a1"] = a1_;
        results_.additionalResults["a2"] = a2_;
        results_.additionalResults["a3"] = a3_;
        results_.additionalResults["a4"] = a4_;
        results_.additionalResults["a5"] = a5_;
    }
}
