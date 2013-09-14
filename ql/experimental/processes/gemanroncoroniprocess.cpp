/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen
 
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

/*! \file gemanroncoroniprocess.cpp
    \brief Geman-Roncoroni process
*/

#include <ql/math/functional.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <ql/experimental/processes/gemanroncoroniprocess.hpp>


namespace QuantLib {

    GemanRoncoroniProcess::GemanRoncoroniProcess(
                                      Real x0, 
                                      Real alpha, Real beta, 
                                      Real gamma, Real delta, 
                                      Real eps, Real zeta, Real d, 
                                      Real k,  Real tau,
                                      Real sig2, Real a, Real b,
                                      Real theta1, Real theta2, Real theta3,
                                      Real psi)
    : StochasticProcess1D(boost::shared_ptr<discretization>(
                                                    new EulerDiscretization)),
      x0_(x0),
      alpha_(alpha), beta_(beta),
      gamma_(gamma), delta_(delta),
      eps_(eps), zeta_(zeta), d_(d), 
      k_(k), tau_(tau),
      sig2_(sig2), a_(a), b_(b),
      theta1_(theta1), theta2_(theta2), theta3_(theta3),
      psi_(psi) {  
    }

    Real GemanRoncoroniProcess::x0() const {
        return x0_;
    }

    Real GemanRoncoroniProcess::drift(Time t, Real x) const {
        const Real mu = alpha_ + beta_*t + gamma_*std::cos(eps_+2*M_PI*t)
                                         + delta_*std::cos(zeta_+4*M_PI*t);
        const Real muPrime = beta_ - gamma_*2*M_PI*std::sin(eps_+2*M_PI*t)
                                   - delta_*4*M_PI*std::sin(zeta_+4*M_PI*t);

        return muPrime + theta1_*(mu-x);
    }
    
    Real GemanRoncoroniProcess::diffusion(Time t, Real /*x*/) const {
        return std::sqrt(sig2_ + a_*square<Real>()(std::cos(M_PI*t+b_)));
    }

    Real GemanRoncoroniProcess::stdDeviation(Time t0, Real /*x0*/, Time dt) const {
        const Volatility sig2t = sig2_+a_*square<Real>()(std::cos(M_PI*t0+b_));
        
        return std::sqrt(sig2t/(2*theta1_)*(1.0-std::exp(-2*theta1_*dt)));
    }

    Real GemanRoncoroniProcess::evolve(Time t0, Real x0, 
                                       Time dt, Real dw) const {
        // random number generator for the jump part 
        if (!urng_) {
            typedef PseudoRandom::urng_type urng_type;
            urng_ = boost::shared_ptr<urng_type>(
                     new urng_type((unsigned long)(1234ul*dw+56789ul)));
        }
        Array du(3); 
        du[0] = urng_->next().value; 
        du[1] = urng_->next().value;

        return evolve(t0, x0, dt, dw, du);
    }

    Real GemanRoncoroniProcess::evolve(Time t0, Real x0, Time dt,
                                       Real dw, const Array& du) const {
        Real retVal;
        const Time t = t0 + 0.5*dt;
        const Real mu = alpha_ + beta_*t + gamma_*std::cos(eps_ +2*M_PI*t)
                                         + delta_*std::cos(zeta_+4*M_PI*t);
        
        const Real j = -1.0/theta3_
                        *std::log(1.0+du[1]*(std::exp(-theta3_*psi_)-1.0));

        if (x0 <= mu+d_) {
            retVal = StochasticProcess1D::evolve(t, x0, dt, dw);
            
            const Real jumpIntensity
                = theta2_*(2.0/(1+std::fabs(std::sin(M_PI*(t-tau_)/k_)))-1);            
            const Time interarrival = -1.0/jumpIntensity*std::log(du[0]);

            if (interarrival < dt) {
                retVal += j;
            }
        }
        else {
            retVal = x0-j;
        }

        return retVal;
    }
}
