/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Klaus Spanderen

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

#include <ql/processes/batesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/poissondistribution.hpp>


namespace QuantLib {
    BatesProcess::BatesProcess(
                              const Handle<YieldTermStructure>& riskFreeRate,
                              const Handle<YieldTermStructure>& dividendYield,
                              const Handle<Quote>& s0,
                              Real v0, Real kappa,
                              Real theta, Real sigma, Real rho,
                              Real lambda, Real nu, Real delta, 
                              HestonProcess::Discretization d)
    : HestonProcess(riskFreeRate, dividendYield, 
                    s0, v0, kappa, theta, sigma, rho, d),
      lambda_(lambda), delta_(delta), nu_(nu),
      m_(std::exp(nu+0.5*delta*delta)-1) {
    }

    Array BatesProcess::drift(Time t, const Array& x) const {
        Array retVal = HestonProcess::drift(t, x);
        retVal[0] -= lambda_*m_;
        return retVal;
    }

    Array BatesProcess::evolve(Time t0, const Array& x0,
                               Time dt, const Array& dw) const {

        const Size hestonFactors = HestonProcess::factors();

        Real p = cumNormalDist_(dw[hestonFactors]);
        if (p<0.0)
            p = 0.0;
        else if (p >= 1.0)
            p = 1.0-QL_EPSILON;
        
        const Real n = InverseCumulativePoisson(lambda_*dt)(p);        
        Array retVal = HestonProcess::evolve(t0, x0, dt, dw);
        retVal[0] *= 
            std::exp(-lambda_*m_*dt + nu_*n+delta_*std::sqrt(n)*dw[hestonFactors+1]);

        return retVal;
    }

    Size BatesProcess::factors() const {
        return HestonProcess::factors() + 2;
    }

    Real BatesProcess::lambda() const {
        return lambda_;
    }

    Real BatesProcess::nu() const {
        return nu_;
    }

    Real BatesProcess::delta() const {
        return delta_;
    }
}
