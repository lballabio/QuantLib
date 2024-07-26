/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020, 2021 Jack Gillett

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

#include <ql/experimental/asian/analytic_discr_geom_av_price_heston.hpp>
#include <utility>

namespace QuantLib {

    // A class to perform the integrations in Eqs (23) and (24)
    class AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::Integrand {
      private:
        Real t_, T_, K_, logK_;
        Size kStar_;
        const std::vector<Time> t_n_, tauK_;
        const AnalyticDiscreteGeometricAveragePriceAsianHestonEngine* const parent_;
        Real xiRightLimit_;
        std::complex<Real> i_;

      public:
        Integrand(Real t,
                  Real T,
                  Size kStar,
                  std::vector<Time> t_n,
                  std::vector<Time> tauK,
                  Real K,
                  const AnalyticDiscreteGeometricAveragePriceAsianHestonEngine* const parent,
                  Real xiRightLimit)
        : t_(t), T_(T), K_(K), logK_(std::log(K)), kStar_(kStar), t_n_(std::move(t_n)),
          tauK_(std::move(tauK)), parent_(parent), xiRightLimit_(xiRightLimit),
          i_(std::complex<Real>(0.0, 1.0)) {}

        Real operator()(Real xi) const {
            Real xiDash = (0.5+1e-8+0.5*xi) * xiRightLimit_; // Map xi to full range

            std::complex<Real> inner1 = parent_->Phi(1.0 + xiDash*i_, 0, t_, T_, kStar_, t_n_, tauK_);
            std::complex<Real> inner2 = -K_*parent_->Phi(xiDash*i_, 0, t_, T_, kStar_, t_n_, tauK_);

            return 0.5*xiRightLimit_*std::real((inner1 + inner2) * std::exp(-xiDash*logK_*i_) / (xiDash*i_));
        }
    };


    AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::
        AnalyticDiscreteGeometricAveragePriceAsianHestonEngine(
            ext::shared_ptr<HestonProcess> process, Real xiRightLimit)
    : process_(std::move(process)), xiRightLimit_(xiRightLimit), integrator_(128) {
        registerWith(process_);

        v0_ = process_->v0();
        rho_ = process_->rho();
        kappa_ = process_->kappa();
        theta_ = process_->theta();
        sigma_ = process_->sigma();
        s0_ = process_->s0();
        logS0_ = std::log(s0_->value());

        riskFreeRate_ = process_->riskFreeRate();
        dividendYield_ = process_->dividendYield();
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::F(
            const std::complex<Real>& z1,
            const std::complex<Real>& z2,
            Time tau) const {
        std::complex<Real> temp = std::sqrt(kappa_*kappa_-2.0*z1*sigma_*sigma_);
        if (std::abs(kappa_*kappa_-2.0*sigma_*sigma_) < 1e-8) {
            return 1.0 + 0.5*(kappa_-z2*sigma_*sigma_);
        } else {
            return cosh(0.5*tau*temp) + (kappa_-z2*sigma_*sigma_)*sinh(0.5*tau*temp)/temp;
        }
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::F_tilde(
            const std::complex<Real>& z1,
            const std::complex<Real>& z2,
            Time tau) const {
        std::complex<Real> temp = std::sqrt(kappa_*kappa_ - 2.0*z1*sigma_*sigma_);
        return 0.5*temp*sinh(0.5*tau*temp) + 0.5*(kappa_ - z2*sigma_*sigma_)*cosh(0.5*tau*temp);
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::z(
            const std::complex<Real>& s, const std::complex<Real>& w, Size k, Size n) const {
        auto k_ = Real(k);
        auto n_ = Real(n);
        std::complex<Real> term1 = (2*rho_*kappa_ - sigma_)*((n_-k_+1)*s + n_*w)/(2*sigma_*n_);
        std::complex<Real> term2 = (1-rho_*rho_)*pow(((n_-k_+1)*s + n_*w), 2)/(2*n_*n_);

        return term1 + term2;
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::omega(
            const std::complex<Real>& s, const std::complex<Real>& w, Size k, Size kStar, Size n) const {
        if (k==kStar) {
            return 0;
        } else if (k==n+1) {
            return rho_*w/sigma_;
        } else {
            return rho_*s/(sigma_*n);
        }
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::a(
            const std::complex<Real>& s,
            const std::complex<Real>& w,
            Time t, Time T, Size kStar,
            const std::vector<Time>& t_n) const {
        auto kStar_ = Real(kStar);
        auto n_ = Real(t_n.size());
        Real temp = -rho_*kappa_*theta_/sigma_;

        Time summation = 0.0;
        Real summation2 = 0.0;
        for (Size i=kStar+1; i<=t_n.size(); i++) {
            summation += t_n[i-1];
            summation2 += tkr_tk_[i-1];
        }
        // This is Eq (16) modified for non-constant rates
        std::complex<Real> term1 = (s*(n_-kStar_)/n_ + w)*(logS0_ - rho_*v0_/sigma_ - t*temp - tr_t_);
        std::complex<Real> term2 = temp*(s*summation/n_ + w*T) + w*Tr_T_ + summation2*s/n_;

        return term1 + term2;
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::omega_tilde(
            const std::complex<Real>& s,
            const std::complex<Real>& w,
            Size k, Size kStar, Size n,
            const std::vector<Time>& tauK) const {
        std::complex<Real> omega_k = omega(s, w, k, kStar, n);
        if (k==n+1) {
            return omega_k;
        } else {
            Time dTauk = tauK[k+1] - tauK[k];
            std::complex<Real> z_kp1 = z(s, w, k+1, n);

            // omega_tilde calls itself recursivly, use lookup map to avoid extreme slowdown when k large
            std::complex<Real> omega_kp1 = 0.0;

            auto position = omegaTildeLookupTable_.find(k+1);

            if (position != omegaTildeLookupTable_.end()) {
                std::complex<Real> value = position->second;
                omega_kp1 = value;
            } else {
                omega_kp1 = omega_tilde(s, w, k+1, kStar, n, tauK);
            }

            std::complex<Real> ratio = F_tilde(z_kp1,omega_kp1,dTauk)/F(z_kp1,omega_kp1,dTauk);
            std::complex<Real> result = omega_k + kappa_/pow(sigma_,2) - 2.0*ratio/pow(sigma_,2);

            // Store this value in our mutable lookup map
            omegaTildeLookupTable_[k] = result;

            return result;
        }
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::Phi(
            const std::complex<Real> s,
            const std::complex<Real> w,
            Time t, Time T, Size kStar,
            const std::vector<Time>& t_n,
            const std::vector<Time>& tauK) const {

        // Clear the mutable lookup map before evaluating Phi
        omegaTildeLookupTable_ = std::map<Size, std::complex<Real> >();

        Size n = t_n.size();
        std::complex<Real> aTerm = a(s, w, t, T, kStar, t_n);
        std::complex<Real> omegaTerm = v0_*omega_tilde(s, w, kStar, kStar, n, tauK);
        Real term3 = kappa_*kappa_*theta_*(T-t)/pow(sigma_,2);

        std::complex<Real> summation = 0.0;
        for (Size i=kStar+1; i<=n+1; i++) {
            Real dTau = tauK[i] - tauK[i-1];
            std::complex<Real> z_k = z(s, w, i, n);
            std::complex<Real> omega_tilde_k = omega_tilde(s, w, i, kStar, n, tauK);

            summation += std::log(F(z_k, omega_tilde_k, dTau));
        }
        std::complex<Real> term4 = 2*kappa_*theta_*summation/pow(sigma_,2);

        return std::exp(aTerm + omegaTerm + term3 - term4);
}

    void AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::calculate() const {
        /* this engine cannot really check for the averageType==Geometric
           since it can be used as control variate for the Arithmetic version
        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");
        */
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        Real runningLog;
        Size pastFixings;
        if (arguments_.averageType == Average::Geometric) {
            QL_REQUIRE(arguments_.runningAccumulator>0.0,
                       "positive running product required: "
                       << arguments_.runningAccumulator << " not allowed");
            runningLog = std::log(arguments_.runningAccumulator);
            pastFixings = arguments_.pastFixings;
        } else {  // it is being used as control variate
            runningLog = 0.0;
            pastFixings = 0;
        }

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real strike = payoff->strike();
        Date exercise = arguments_.exercise->lastDate();

        Time expiryTime = this->process_->time(exercise);
        QL_REQUIRE(expiryTime >= 0.0, "Expiry Date cannot be in the past");

        Real expiryDcf = riskFreeRate_->discount(expiryTime);

        Time startTime = 0.0;
        std::vector<Time> fixingTimes, tauK;
        for (auto& fixingDate : arguments_.fixingDates) {
            fixingTimes.push_back(this->process_->time(fixingDate));
        }
        std::sort(fixingTimes.begin(), fixingTimes.end());
        tauK = fixingTimes;

        // tauK is just a vector of the sorted future fixing times (ie. from the kStar element
        // onwards), with t pushed on the front and T pushed on the back!
        tauK.insert(tauK.begin(), startTime);
        tauK.push_back(expiryTime);

        // In the paper, seasoned asians are dealt with by letting the start time variable be greater
        // than 0. We can achieve the same by fixing the start time to 0.0, but attaching 'dummy'
        // fixing times at t=-1 for each past fixing, at the front of the fixing times arrays
        for (Size i=0; i<pastFixings; i++) {
            fixingTimes.insert(fixingTimes.begin(), -1.0);
            tauK.insert(tauK.begin(), -1.0);
        }

        Size kStar = pastFixings;

        // Need the log of some discount factors to calculate the r-adjusted a factor (Eq 16)
        tr_t_ = 0;
        Tr_T_ = 0;
        tkr_tk_ = std::vector<Real>();
        tr_t_ = -std::log(riskFreeRate_->discount(startTime) / dividendYield_->discount(startTime));
        Tr_T_ = -std::log(riskFreeRate_->discount(expiryTime) / dividendYield_->discount(expiryTime));
        for (Real fixingTime : fixingTimes) {
            if (fixingTime < 0) {
                tkr_tk_.push_back(1.0);
            } else {
                tkr_tk_.push_back(-std::log(riskFreeRate_->discount(fixingTime) /
                                            dividendYield_->discount(fixingTime)));
            }
        }

        // To account for seasoning, we need to calculate an 'adjusted' strike (Eq 6)
        Real prefactor = std::exp(runningLog / fixingTimes.size());
        Real adjustedStrike = strike / prefactor;

        // Calculate the two terms in eq (23) - Phi(1,0) is real (asian forward) but need to type convert
        Real term1 = 0.5 * (std::real(Phi(1,0, startTime, expiryTime, kStar, fixingTimes, tauK)) - adjustedStrike);

        Integrand integrand(startTime, expiryTime, kStar, fixingTimes, tauK, adjustedStrike, this, xiRightLimit_);
        Real term2 = integrator_(integrand) / M_PI;

        // Apply the payoff functions
        Real value = 0.0;
        switch (payoff->optionType()){
            case Option::Call:
                value = expiryDcf * prefactor * (term1 + term2);
                break;
            case Option::Put:
                value = expiryDcf * prefactor * (-term1 + term2);
                break;
            default:
                QL_FAIL("unknown option type");
            }

        results_.value = value;

        results_.additionalResults["dcf"] = expiryDcf;
        results_.additionalResults["s0"] = s0_->value();
        results_.additionalResults["strike"] = strike;
        results_.additionalResults["expiryTime"] = expiryTime;
        results_.additionalResults["term1"] = term1;
        results_.additionalResults["term2"] = term2;
        results_.additionalResults["xiRightLimit"] = xiRightLimit_;
        results_.additionalResults["fixingTimes"] = fixingTimes;
        results_.additionalResults["tauK"] = tauK;
        results_.additionalResults["adjustedStrike"] = adjustedStrike;
        results_.additionalResults["prefactor"] = prefactor;
        results_.additionalResults["kStar"] = kStar;
    }
}
