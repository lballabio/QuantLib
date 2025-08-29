/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Banca Profilo S.p.A.

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

#include <ql/processes/hullwhiteprocess.hpp>

namespace QuantLib {

    HullWhiteProcess::HullWhiteProcess(const Handle<YieldTermStructure>& h,
                                       Real a,
                                       Real sigma)
    : process_(new OrnsteinUhlenbeckProcess(
                   a, sigma, h->forwardRate(0.0,0.0,Continuous,NoFrequency))),
      h_(h), a_(a), sigma_(sigma) {
        QL_REQUIRE(a_ >= 0.0, "negative a given");
        QL_REQUIRE(sigma_ >= 0.0, "negative sigma given");
    }

    Real HullWhiteProcess::x0() const {
        return process_->x0();
    }

    Real HullWhiteProcess::drift(Time t, Real x) const {
        Real alpha_drift = sigma_*sigma_/(2*a_)*(1-std::exp(-2*a_*t));
        Real shift = 0.0001;
        Real f = h_->forwardRate(t, t, Continuous, NoFrequency);
        Real fup = h_->forwardRate(t+shift, t+shift, Continuous, NoFrequency);
        Real f_prime = (fup-f)/shift;
        alpha_drift += a_*f+f_prime;
        return process_->drift(t, x) + alpha_drift;
    }

    Real HullWhiteProcess::diffusion(Time t, Real x) const{
        return process_->diffusion(t, x);
    }

    Real HullWhiteProcess::expectation(Time t0, Real x0, Time dt) const {
        return process_->expectation(t0, x0, dt)
             + alpha(t0 + dt) - alpha(t0)*std::exp(-a_*dt);
    }

    Real HullWhiteProcess::stdDeviation(Time t0, Real x0, Time dt) const{
        return process_->stdDeviation(t0, x0, dt);
    }

    Real HullWhiteProcess::variance(Time t0, Real x0, Time dt) const{
        return process_->variance(t0, x0, dt);
    }

    Real HullWhiteProcess::alpha(Time t) const {
        Real alfa = a_ > QL_EPSILON ?
                    Real((sigma_/a_)*(1 - std::exp(-a_*t))) :
                    sigma_*t;
        alfa *= 0.5*alfa;
        alfa += h_->forwardRate(t, t, Continuous, NoFrequency);
        return alfa;
    }

    Real HullWhiteProcess::a() const {
        return a_;
    }

    Real HullWhiteProcess::sigma() const {
        return sigma_;
    }

    HullWhiteForwardProcess::HullWhiteForwardProcess(
                                          const Handle<YieldTermStructure>& h,
                                          Real a,
                                          Real sigma)
    : process_(new OrnsteinUhlenbeckProcess(
                   a, sigma, h->forwardRate(0.0,0.0,Continuous,NoFrequency))),
      h_(h), a_(a), sigma_(sigma) {}

    Real HullWhiteForwardProcess::x0() const {
        return process_->x0();
    }

    Real HullWhiteForwardProcess::drift(Time t, Real x) const {
        Real alpha_drift = sigma_*sigma_/(2*a_)*(1-std::exp(-2*a_*t));
        Real shift = 0.0001;
        Real f = h_->forwardRate(t, t, Continuous, NoFrequency);
        Real fup = h_->forwardRate(t+shift, t+shift, Continuous, NoFrequency);
        Real f_prime = (fup-f)/shift;
        alpha_drift += a_*f+f_prime;
        return process_->drift(t, x) + alpha_drift - B(t, T_)*sigma_*sigma_;
    }

    Real HullWhiteForwardProcess::diffusion(Time t, Real x) const{
        return process_->diffusion(t, x);
    }

    Real HullWhiteForwardProcess::expectation(Time t0, Real x0,
                                              Time dt) const {
        return process_->expectation(t0, x0, dt)
             + alpha(t0 + dt) - alpha(t0)*std::exp(-a_*dt)
             - M_T(t0, t0+dt, T_);
    }

    Real HullWhiteForwardProcess::stdDeviation(Time t0, Real x0,
                                               Time dt) const {
        return process_->stdDeviation(t0, x0, dt);
    }

    Real HullWhiteForwardProcess::variance(Time t0, Real x0, Time dt) const{
        return process_->variance(t0, x0, dt);
    }

    Real HullWhiteForwardProcess::alpha(Time t) const {
        Real alfa = a_ > QL_EPSILON ?
                    Real((sigma_/a_)*(1 - std::exp(-a_*t))) :
                    sigma_*t;
        alfa *= 0.5*alfa;
        alfa += h_->forwardRate(t, t, Continuous, NoFrequency);

        return alfa;
    }

    Real HullWhiteForwardProcess::M_T(Real s, Real t, Real T) const {
        if (a_ > QL_EPSILON) {
            Real coeff = (sigma_*sigma_)/(a_*a_);
            Real exp1 = std::exp(-a_*(t-s));
            Real exp2 = std::exp(-a_*(T-t));
            Real exp3 = std::exp(-a_*(T+t-2.0*s));
            return coeff*(1-exp1)-0.5*coeff*(exp2-exp3);
        } else {
            // low-a algebraic limit
            Real coeff = (sigma_*sigma_)/2.0;
            return coeff*(t-s)*(2.0*T-t-s);
        }
    }

    Real HullWhiteForwardProcess::B(Time t, Time T) const {
        return a_ > QL_EPSILON ?
               Real(1/a_ * (1-std::exp(-a_*(T-t)))) :
               T-t;
    }

    Real HullWhiteForwardProcess::a() const {
        return a_;
    }

    Real HullWhiteForwardProcess::sigma() const {
        return sigma_;
    }
}

