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

#include <ql/experimental/asian/analytic_discr_geom_av_price_heston.hpp>

#include <iostream>
#include <iomanip>

namespace QuantLib {

    AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::
        AnalyticDiscreteGeometricAveragePriceAsianHestonEngine(
            const ext::shared_ptr<HestonProcess>& process)
    : r0_(0.0), process_(process), integrator_(128) {
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
            std::complex<Real>& z1,
            std::complex<Real>& z2,
            Time tau) const {
        std::complex<Real> temp = std::sqrt(kappa_*kappa_-2.0*z1*sigma_*sigma_);
        if (std::abs(kappa_*kappa_-2.0*sigma_*sigma_) < 1e-8) {
            return 1.0 + 0.5*(kappa_-z2*sigma_*sigma_);
        } else {
            return cosh(0.5*tau*temp) + (kappa_-z2*sigma_*sigma_)*sinh(0.5*tau*temp)/temp;
        }
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::F_tilde(
            std::complex<Real>& z1,
            std::complex<Real>& z2,
            Time tau) const {
        std::complex<Real> temp = std::sqrt(kappa_*kappa_ - 2.0*z1*sigma_*sigma_);
        return 0.5*temp*sinh(0.5*tau*temp) + 0.5*(kappa_ - z2*sigma_*sigma_)*cosh(0.5*tau*temp);
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::z(
            std::complex<Real>& s, std::complex<Real>& w, Size k, Size n) const {
        double k_ = double(k);
        double n_ = double(n);
        std::complex<Real> term1 = (2*rho_*kappa_ - sigma_)*((n_-k_+1)*s + n_*w)/(2*sigma_*n_);
        std::complex<Real> term2 = (1-rho_*rho_)*pow(((n_-k_+1)*s + n_*w), 2)/(2*n_*n_);

        return term1 + term2;
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::omega(
            std::complex<Real>& s, std::complex<Real>& w, Size k, Size kStar, Size n) const {
        if (k==kStar) {
            return 0;
        } else if (k==n+1) {
            return rho_*w/sigma_;
        } else {
            return rho_*s/(sigma_*n);
        }
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::a(
            std::complex<Real>& s,
            std::complex<Real>& w,
            Time t, Time T, Size kStar, std::vector<Time>& t_n) const {
        double kStar_ = double(kStar);
        double n_ = double(t_n.size());
        Real temp = (r0_ - rho_*kappa_*theta_/sigma_);

        Time summation = 0.0;
        for (Size i=kStar+1; i<=t_n.size(); i++) {
            summation += t_n[i];
        }

        std::complex<Real> term1 = (s*(n_-kStar_)/n_ + w)*(logS0_ - rho_*v0_/sigma_ - t*temp);
        std::complex<Real> term2 = temp*(s*summation/n_ + w*T);

        return term1 + term2;
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::omega_tilde(
            std::complex<Real>& s,
            std::complex<Real>& w,
            Size k, Size kStar, Size n, std::vector<Time>& tauK) const {
        std::complex<Real> omega_k = omega(s, w, k, kStar, n);
        if (k==n+1) {
            return omega_k;
        } else {
            Time dTauk = tauK[k+1] - tauK[k];
            std::complex<Real> z_kp1 = z(s, w, k+1, n);

            // omega_tilde calls itself recursivly, so use a lookup map to avoid exponential slowdown
            std::complex<Real> omega_kp1 = 0.0;
            std::tuple<Size, Real, Real, Real, Real> location = 
                std::tuple<Size, Real, Real, Real, Real>(k+1,
                    std::real(s), std::imag(s), std::real(w), std::imag(w));
            std::map<std::tuple<Size, Real, Real, Real, Real>,
                std::complex<Real> >::const_iterator position = omegaTildeLookupTable_.find(location);

            if (position != omegaTildeLookupTable_.end()) {
                std::complex<Real> value = position->second;
                omega_kp1 = value;
            } else {
                omega_kp1 = omega_tilde(s, w, k+1, kStar, n, tauK);
            }

            std::complex<Real> ratio = F_tilde(z_kp1,omega_kp1,dTauk)/F_tilde(z_kp1,omega_kp1,dTauk);

            return omega_k + kappa_/pow(sigma_,2) - 0.5*ratio/pow(sigma_,2);
        }
    }

    std::complex<Real> AnalyticDiscreteGeometricAveragePriceAsianHestonEngine::Phi(
            std::complex<Real>& s,
            std::complex<Real>& w,
            Time t, Time T, Size kStar, std::vector<Time>& t_n, std::vector<Time>& tauK) const {
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

        std::vector<Time> fixingTimes, tauK;
        for (Size i=0; i<arguments_.fixingDates.size(); i++) {
            fixingTimes.push_back(this->process_->time(arguments_.fixingDates[i]));
        }
        std::sort(fixingTimes.begin(), fixingTimes.end());


        // TODO: extend to cover seasoned options (discussed in paper)
        Time startTime = 0.0;

        Size kStar = 0;
        for (Size i=0; i<fixingTimes.size(); i++) {
            if (startTime > fixingTimes[i]) {
                kStar = i+1;
            } else {
                tauK.push_back(this->process_->time(arguments_.fixingDates[i]));
            }
        }

        // tauK is just a vector of the sorted fixing times from the kStar element
        // onwards, with t pushed on the front and T pushed on the back!
        tauK.insert(tauK.begin(), startTime);
        tauK.push_back(expiryTime);


        // Apply the payoff functions
        Real value = 0.0;
        switch (payoff->optionType()){
            case Option::Call:
                value = 100.0;
                break;
            case Option::Put:
                value = 100.0;
                break;
            default:
                QL_FAIL("unknown option type");
            }

        results_.value = value;
    }
}

