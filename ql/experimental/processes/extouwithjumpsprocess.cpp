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

/*! \file expouwithjumpsprocess.cpp
    \brief Ornstein Uhlenbeck process plus exp jumps (Kluge Model)
*/

#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <utility>

namespace QuantLib {

    ExtOUWithJumpsProcess::ExtOUWithJumpsProcess(
        ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> process,
        Real Y0,
        Real beta,
        Real jumpIntensity,
        Real eta)
    : Y0_(Y0), beta_(beta), jumpIntensity_(jumpIntensity), eta_(eta),
      ouProcess_(std::move(process)) {}

    Size ExtOUWithJumpsProcess::size() const {
        return 2;
    }    
    Size ExtOUWithJumpsProcess::factors() const {
        return 3;
    }
    ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>
        ExtOUWithJumpsProcess::getExtendedOrnsteinUhlenbeckProcess() const {
        return ouProcess_;
    }
    Real ExtOUWithJumpsProcess::beta() const {
        return beta_;
    }
    Real ExtOUWithJumpsProcess::jumpIntensity() const {
        return jumpIntensity_;
    }
    Real ExtOUWithJumpsProcess::eta() const {
        return eta_;
    }

    Disposable<Array> ExtOUWithJumpsProcess::initialValues() const {
        Array retVal(2);
        retVal[0] = ouProcess_->x0();
        retVal[1] = Y0_;
        
        return retVal;
    }
    
    Disposable<Array> ExtOUWithJumpsProcess::drift(Time t, const Array& x) const {
        Array retVal(2);
        retVal[0] = ouProcess_->drift(t, x[0]);
        retVal[1] = -beta_*x[1];
        
        return retVal;
    }

    Disposable<Matrix> 
    ExtOUWithJumpsProcess::diffusion(Time t, const Array& x) const {
        Matrix retVal(2, 2, 0.0);    
        retVal[0][0] = ouProcess_->diffusion(t, x[0]);
        
        return retVal;
    }

    Disposable<Array> ExtOUWithJumpsProcess::evolve(
        Time t0, const Array& x0, Time dt, const Array& dw) const {
        
        Array retVal(2);
        retVal[0] = ouProcess_->evolve(t0, x0[0], dt, dw[0]);
        retVal[1] = x0[1]*std::exp(-beta_*dt);
                
        const Real u1 = std::max(QL_EPSILON, std::min(cumNormalDist_(dw[1]), 
                                                      1.0-QL_EPSILON));

        const Time interarrival = -1.0/jumpIntensity_*std::log(u1);
        if (interarrival < dt) {
            const Real u2 = std::max(QL_EPSILON, std::min(cumNormalDist_(dw[2]), 
                                                          1.0-QL_EPSILON));
            const Real jumpSize = -1.0/eta_*std::log(u2);
            retVal[1] += jumpSize;
        }
        return retVal;
    }
}
