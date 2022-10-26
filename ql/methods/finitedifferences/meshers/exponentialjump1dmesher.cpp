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

/*! \file exponentialjump1dmesher.cpp
    \brief mesher for a exponential jump mesher with high 
           mean reversion rate and low jump intensity
*/

#include <ql/math/incompletegamma.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/methods/finitedifferences/meshers/exponentialjump1dmesher.hpp>
#include <algorithm>

namespace QuantLib {
    ExponentialJump1dMesher::ExponentialJump1dMesher(
          Size steps, Real beta, Real jumpIntensity, Real eta, Real eps)
    : Fdm1dMesher(steps),
      beta_(beta), jumpIntensity_(jumpIntensity), eta_(eta)
   {
        QL_REQUIRE(eps > 0.0 && eps < 1.0, "eps > 0.0 and eps < 1.0");
        QL_REQUIRE(steps > 1, "minimum number of steps is two");
        
        const Real start = 0.0;
        const Real end   = 1.0-eps;
        const Real dx    = (end-start)/(steps-1);
        const Real scale = 1/(1-std::exp(-beta/jumpIntensity));

        for (Size i=0; i < steps; ++i) {
            const Real p = start + i*dx;
            locations_[i] = scale*(-1.0/eta*std::log(1.0-p));
        }

        for (Size i=0; i < steps-1; ++i) {
            dminus_[i+1] = dplus_[i] = locations_[i+1]-locations_[i];
        }
        dplus_.back() = dminus_.front() = Null<Real>();
    }
                                    
                                    
    Real ExponentialJump1dMesher::jumpSizeDensity(Real x, Time t) const {
        const Real a    = 1.0-jumpIntensity_/beta_;
        const Real norm = 1.0-std::exp(-jumpIntensity_*t);
        const Real gammaValue 
            = std::exp(GammaFunction().logValue(1.0-jumpIntensity_/beta_));
        return jumpIntensity_*gammaValue/norm
                    *( incompleteGammaFunction(a, x*std::exp(beta_*t)*eta_)
                      -incompleteGammaFunction(a, x*eta_))
                    *std::pow(eta_, jumpIntensity_/beta_)
                    /(beta_*std::pow(x, a));
    }
    
    Real ExponentialJump1dMesher::jumpSizeDensity(Real x) const {
        const Real a = 1.0-jumpIntensity_/beta_;
        const Real gammaValue 
                = std::exp(GammaFunction().logValue(jumpIntensity_/beta_));
        return std::exp(-x*eta_)*std::pow(x, -a) * std::pow(eta_, 1.0-a) 
                / gammaValue;
    }

    Real ExponentialJump1dMesher::jumpSizeDistribution(Real x, Time t) const {
        const Real xmin = std::min(x, 1.0e-100);

        return GaussLobattoIntegral(1000000, 1e-12)(
            [&](Real _x){ return jumpSizeDensity(_x, t); },
            xmin, std::max(x, xmin));
    }

    Real ExponentialJump1dMesher::jumpSizeDistribution(Real x) const {
        const Real a    = jumpIntensity_/beta_;
        const Real xmin = std::min(x, QL_EPSILON);
        const Real gammaValue 
                = std::exp(GammaFunction().logValue(jumpIntensity_/beta_));
        
        const Real lowerEps = 
            (std::pow(xmin, a)/a - std::pow(xmin, a+1)/(a+1))/gammaValue;
        
        return lowerEps + GaussLobattoIntegral(10000, 1e-12)(
            [&](Real _x){ return jumpSizeDensity(_x); },
            xmin/eta_, std::max(x, xmin/eta_));
    }
}
