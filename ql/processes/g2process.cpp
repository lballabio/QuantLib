/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/processes/g2process.hpp>
#include <ql/processes/eulerdiscretization.hpp>

namespace QuantLib {

    G2Process::G2Process(const Handle<YieldTermStructure>& h, Real a, Real sigma, Real b, Real eta, Real rho)
    : a_(a), sigma_(sigma), b_(b), eta_(eta), rho_(rho),
      xProcess_(new QuantLib::OrnsteinUhlenbeckProcess(a, sigma, 0.0)),
      yProcess_(new QuantLib::OrnsteinUhlenbeckProcess(b, eta, 0.0)), h_(h) {}

    Size G2Process::size() const {
        return 2;
    }

    Array G2Process::initialValues() const {
        return { x0_, y0_ };
    }

    Array G2Process::drift(Time t, const Array& x) const {
        Real alpha_drift = sigma_*sigma_/(2*a_)*(1-std::exp(-2*a_*t));
        Real shift = 0.0001;
        Real f = h_->forwardRate(t, t, Continuous, NoFrequency);
        Real fup = h_->forwardRate(t+shift, t+shift, Continuous, NoFrequency);
        Real f_prime = (fup-f)/shift;
        alpha_drift += a_*f+f_prime;

        return {
            xProcess_->drift(t, x[0]) + alpha_drift,
            yProcess_->drift(t, x[1]) + alpha_drift
        };
    }

    Matrix G2Process::diffusion(Time, const Array&) const {
        /* the correlation matrix is
           |  1   rho |
           | rho   1  |
           whose square root (which is used here) is
           |  1          0       |
           | rho   sqrt(1-rho^2) |
        */
        Matrix tmp(2,2);
        Real sigma1 = sigma_;
        Real sigma2 = eta_;
        tmp[0][0] = sigma1;       tmp[0][1] = 0.0;
        tmp[1][0] = rho_*sigma1;  tmp[1][1] = std::sqrt(1.0-rho_*rho_)*sigma2;
        return tmp;
    }

    Array G2Process::expectation(Time t0, const Array& x0,
                                 Time dt) const {
        return {
            xProcess_->expectation(t0, x0[0], dt),
            yProcess_->expectation(t0, x0[1], dt)
        };
    }

    Matrix G2Process::stdDeviation(Time t0, const Array& x0, Time dt) const {
        /* the correlation matrix is
           |  1   rho |
           | rho   1  |
           whose square root (which is used here) is
           |  1          0       |
           | rho   sqrt(1-rho^2) |
        */
        Matrix tmp(2,2);
        Real sigma1 = xProcess_->stdDeviation(t0, x0[0], dt);
        Real sigma2 = yProcess_->stdDeviation(t0, x0[1], dt);
        Real expa = std::exp(-a_*dt), expb = std::exp(-b_*dt);
        Real H = (rho_*sigma_*eta_)/(a_+b_)*(1-expa*expb);
        Real den =
            (0.5*sigma_*eta_)*std::sqrt((1-expa*expa)*(1-expb*expb)/(a_*b_));
        Real newRho = H/den;
        tmp[0][0] = sigma1;
        tmp[0][1] = 0.0;
        tmp[1][0] = newRho*sigma2;
        tmp[1][1] = std::sqrt(1.0-newRho*newRho)*sigma2;
        return tmp;
    }

    Matrix G2Process::covariance(Time t0, const Array& x0, Time dt) const {
        Matrix sigma = stdDeviation(t0, x0, dt);
        Matrix result = sigma*transpose(sigma);
        return result;
    }

    Real G2Process::x0() const {
        return x0_;
    }

    Real G2Process::y0() const {
        return y0_;
    }

    Real G2Process::a() const {
        return a_;
    }

    Real G2Process::sigma() const {
        return sigma_;
    }

    Real G2Process::b() const {
        return b_;
    }

    Real G2Process::eta() const {
        return eta_;
    }

    Real G2Process::rho() const {
        return rho_;
    }


    G2ForwardProcess::G2ForwardProcess(const Handle<YieldTermStructure>& h, Real a, Real sigma, Real b, Real eta, Real rho)
    : a_(a), sigma_(sigma), b_(b), eta_(eta), rho_(rho),
      xProcess_(new QuantLib::OrnsteinUhlenbeckProcess(a, sigma, 0.0)),
      yProcess_(new QuantLib::OrnsteinUhlenbeckProcess(b, eta, 0.0)),h_(h) {}

    Size G2ForwardProcess::size() const {
        return 2;
    }

    Array G2ForwardProcess::initialValues() const {
        return { x0_, y0_ };
    }

    Array G2ForwardProcess::drift(Time t, const Array& x) const {
        return {
            xProcess_->drift(t, x[0]) + xForwardDrift(t, T_),
            yProcess_->drift(t, x[1]) + yForwardDrift(t, T_)
        };
    }

    Matrix G2ForwardProcess::diffusion(Time, const Array&) const {
        Matrix tmp(2,2);
        Real sigma1 = sigma_;
        Real sigma2 = eta_;
        tmp[0][0] = sigma1;       tmp[0][1] = 0.0;
        tmp[1][0] = rho_*sigma1;  tmp[1][1] = std::sqrt(1.0-rho_*rho_)*sigma2;
        return tmp;
    }

    Array G2ForwardProcess::expectation(Time t0, const Array& x0,
                                        Time dt) const {
        return {
            xProcess_->expectation(t0, x0[0], dt) - Mx_T(t0, t0+dt, T_),
            yProcess_->expectation(t0, x0[1], dt) - My_T(t0, t0+dt, T_)
        };
    }

    Matrix G2ForwardProcess::stdDeviation(Time t0, const Array& x0, Time dt) const {
        Matrix tmp(2,2);
        Real sigma1 = xProcess_->stdDeviation(t0, x0[0], dt);
        Real sigma2 = yProcess_->stdDeviation(t0, x0[1], dt);
        Real expa = std::exp(-a_*dt), expb = std::exp(-b_*dt);
        Real H = (rho_*sigma_*eta_)/(a_+b_)*(1-expa*expb);
        Real den =
            (0.5*sigma_*eta_)*std::sqrt((1-expa*expa)*(1-expb*expb)/(a_*b_));
        Real newRho = H/den;
        tmp[0][0] = sigma1;
        tmp[0][1] = 0.0;
        tmp[1][0] = newRho*sigma2;
        tmp[1][1] = std::sqrt(1.0-newRho*newRho)*sigma2;
        return tmp;
    }

    Matrix G2ForwardProcess::covariance(Time t0, const Array& x0, Time dt) const {
        Matrix sigma = stdDeviation(t0, x0, dt);
        Matrix result = sigma*transpose(sigma);
        return result;
    }

    Real G2ForwardProcess::xForwardDrift(Time t, Time T) const {
        Real expatT = std::exp(-a_*(T-t));
        Real expbtT = std::exp(-b_*(T-t));

        return -(sigma_*sigma_/a_) * (1-expatT)
              - (rho_*sigma_*eta_/b_) * (1-expbtT);
    }

    Real G2ForwardProcess::yForwardDrift(Time t, Time T) const {
        Real expatT = std::exp(-a_*(T-t));
        Real expbtT = std::exp(-b_*(T-t));

        return -(eta_*eta_/b_) * (1-expbtT)
              - (rho_*sigma_*eta_/a_) * (1-expatT);
    }

    Real G2ForwardProcess::Mx_T(Real s, Real t, Real T) const {
        Real M;
        M = ( (sigma_*sigma_)/(a_*a_) + (rho_*sigma_*eta_)/(a_*b_) )
          * (1-std::exp(-a_*(t-s)));
        M += -(sigma_*sigma_)/(2*a_*a_) *
              (std::exp(-a_*(T-t))-std::exp(-a_*(T+t-2*s)));
        M += -(rho_*sigma_*eta_)/(b_*(a_+b_))
            * (std::exp(-b_*(T-t)) -std::exp(-b_*T-a_*t+(a_+b_)*s));
        return M;
    }

    Real G2ForwardProcess::My_T(Real s, Real t, Real T) const {
        Real M;
        M = ( (eta_*eta_)/(b_*b_) + (rho_*sigma_*eta_)/(a_*b_) )
          * (1-std::exp(-b_*(t-s)));
        M += -(eta_*eta_)/(2*b_*b_) *
              (std::exp(-b_*(T-t))-std::exp(-b_*(T+t-2*s)));
        M += -(rho_*sigma_*eta_)/(a_*(a_+b_))
            * (std::exp(-a_*(T-t))-std::exp(-a_*T-b_*t+(a_+b_)*s));
        return M;
    }

}

