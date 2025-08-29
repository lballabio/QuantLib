/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

/*! \file hybridhestonhullwhiteprocess.hpp
    \brief hybrid equity (heston model)
           with stochastic interest rates (hull white model)
*/

#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>

namespace QuantLib {

    HybridHestonHullWhiteProcess::HybridHestonHullWhiteProcess(
        const ext::shared_ptr<HestonProcess> & hestonProcess,
        const ext::shared_ptr<HullWhiteForwardProcess> & hullWhiteProcess,
        Real corrEquityShortRate,
        HybridHestonHullWhiteProcess::Discretization discretization)
    : hestonProcess_(hestonProcess),
      hullWhiteProcess_(hullWhiteProcess),
      hullWhiteModel_(new HullWhite(hestonProcess->riskFreeRate(),
                                    hullWhiteProcess->a(),
                                    hullWhiteProcess->sigma())),
      corrEquityShortRate_(corrEquityShortRate),
      discretization_(discretization),
      maxRho_(std::sqrt(1-hestonProcess->rho()*hestonProcess->rho()) 
              - std::sqrt(QL_EPSILON) /* reserve for rounding errors */),

      T_(hullWhiteProcess->getForwardMeasureTime()),
      endDiscount_(hestonProcess->riskFreeRate()->discount(T_)) {

        QL_REQUIRE(  corrEquityShortRate*corrEquityShortRate
                    +hestonProcess->rho()*hestonProcess->rho() <= 1.0,
                    "correlation matrix is not positive definite");
        
        QL_REQUIRE(hullWhiteProcess->sigma() > 0.0, 
                   "positive vol of Hull White process is required");
    }
        
    Size HybridHestonHullWhiteProcess::size() const {
        return 3;
    }

    Array HybridHestonHullWhiteProcess::initialValues() const {
        return {
            hestonProcess_->s0()->value(),
            hestonProcess_->v0(),
            hullWhiteProcess_->x0()
        };
    }

    Array HybridHestonHullWhiteProcess::drift(Time t, const Array& x) const {
        Array x0 = { x[0], x[1] };
        Array y0 = hestonProcess_->drift(t, x0);

        return {
            y0[0],
            y0[1],
            hullWhiteProcess_->drift(t, x[2])
        };
    }

    Array HybridHestonHullWhiteProcess::apply(const Array& x0, const Array& dx) const {
        Array xt = { x0[0], x0[1] }, dxt = { dx[0], dx[1] };
        Array yt = hestonProcess_->apply(xt, dxt);

        return {
            yt[0],
            yt[1],
            hullWhiteProcess_->apply(x0[2], dx[2])
        };
    }
    
    Matrix HybridHestonHullWhiteProcess::diffusion(Time t, const Array& x) const {
        Matrix retVal(3,3);

        Array xt(2); xt[0] = x[0]; xt[1] = x[1];
        Matrix m = hestonProcess_->diffusion(t, xt);
        retVal[0][0] = m[0][0]; retVal[0][1] = 0.0;     retVal[0][2] = 0.0;
        retVal[1][0] = m[1][0]; retVal[1][1] = m[1][1]; retVal[1][2] = 0.0;
        
        const Real sigma = hullWhiteProcess_->sigma();
        retVal[2][0] = corrEquityShortRate_ * sigma;
        retVal[2][1] = - retVal[2][0]*retVal[1][0] / retVal[1][1];
        retVal[2][2] = std::sqrt( sigma*sigma - retVal[2][1]*retVal[2][1] 
                                              - retVal[2][0]*retVal[2][0] );
        
        return retVal;
    }

    Array HybridHestonHullWhiteProcess::evolve(Time t0, const Array& x0,
                                               Time dt, const Array& dw) const {

        const Rate r         = x0[2];
        const Real a         = hullWhiteProcess_->a();
        const Real sigma     = hullWhiteProcess_->sigma();
        const Real rho       = corrEquityShortRate_;
        const Real xi        = hestonProcess_->rho();
        const Volatility eta = (x0[1] > 0.0) ? Real(std::sqrt(x0[1])) : 0.0;
        const Time s = t0;
        const Time t = t0 + dt;
        const Time T = T_;
        const Rate dy
            = hestonProcess_->dividendYield()->forwardRate(s, t, Continuous,
                                                           NoFrequency);

        const Real df
            = std::log(  hestonProcess_->riskFreeRate()->discount(t)
                       / hestonProcess_->riskFreeRate()->discount(s));

        const Real eaT=std::exp(-a*T);
        const Real eat=std::exp(-a*t);
        const Real eas=std::exp(-a*s);
        const Real iat=1.0/eat;
        const Real ias=1.0/eas;

        const Real m1 = -(dy+0.5*eta*eta)*dt - df;

        const Real m2 = -rho*sigma*eta/a*(dt-1/a*eaT*(iat-ias));

        const Real m3 = (r - hullWhiteProcess_->alpha(s))
                       *hullWhiteProcess_->B(s,t);

        const Real m4 = sigma*sigma/(2*a*a)
            *(dt + 2/a*(eat-eas) - 1/(2*a)*(eat*eat-eas*eas));

        const Real m5 = -sigma*sigma/(a*a)
            *(dt - 1/a*(1-eat*ias) - 1/(2*a)*eaT*(iat-2*ias+eat*ias*ias));

        const Real mu = m1 + m2 + m3 + m4 + m5;

        Array retVal(3);
        
        const Real eta2 = hestonProcess_->sigma() * eta;
        const Real nu
            = hestonProcess_->kappa()*(hestonProcess_->theta() - eta*eta);

        retVal[1] = x0[1] + nu*dt + eta2*std::sqrt(dt)
                                          *(xi*dw[0]+std::sqrt(1-xi*xi)*dw[1]);

        if (discretization_ == BSMHullWhite) {
            const Real v1 = eta*eta*dt 
                + sigma*sigma/(a*a)*(dt - 2/a*(1 - eat*ias) 
                                        + 1/(2*a)*(1 - eat*eat*ias*ias))
                + 2*sigma*eta/a*rho*(dt - 1/a*(1 - eat*ias));
            const Real v2 = hullWhiteProcess_->variance(t0, r, dt);
            const Real v12 = (1-eat*ias)*(sigma*eta/a*rho + sigma*sigma/(a*a))
                            - sigma*sigma/(2*a*a)*(1 - eat*eat*ias*ias);
    
            QL_REQUIRE(v1 > 0.0 && v2 > 0.0, "zero or negative variance given");
            
            // terminal rho must be between -maxRho and +maxRho
            const Real rhoT 
                = std::min(maxRho_, std::max(-maxRho_, v12/std::sqrt(v1*v2)));
            QL_REQUIRE(    rhoT <= 1.0 && rhoT >= -1.0
                       && 1-rhoT*rhoT/(1-xi*xi) >= 0.0, 
                       "invalid terminal correlation");
            
            const Real dw_0 =  dw[0];
            const Real dw_2 =  rhoT*dw[0]- rhoT*xi/std::sqrt(1-xi*xi)*dw[1] 
                             + std::sqrt(1 - rhoT*rhoT/(1-xi*xi))*dw[2];        
    
            retVal[2] = hullWhiteProcess_->evolve(t0, r, dt, dw_2);
    
            const Real vol = std::sqrt(v1)*dw_0;
            retVal[0] = x0[0]*std::exp(mu + vol);
        }
        else if (discretization_ == Euler) {
            const Real dw_2 =  rho*dw[0]- rho*xi/std::sqrt(1-xi*xi)*dw[1] 
                             + std::sqrt(1 - rho*rho/(1-xi*xi))*dw[2];        
    
            retVal[2] = hullWhiteProcess_->evolve(t0, r, dt, dw_2);
    
            const Real vol = eta*std::sqrt(dt)*dw[0];
            retVal[0] = x0[0]*std::exp(mu + vol);            
        }
        else
            QL_FAIL("unknown discretization scheme");

        return retVal;
    }
    
    DiscountFactor
    HybridHestonHullWhiteProcess::numeraire(Time t, const Array& x) const {

        return hullWhiteModel_->discountBond(t, T_, x[2]) / endDiscount_;
    }

    Real HybridHestonHullWhiteProcess::eta() const {
        return corrEquityShortRate_;
    }

    const ext::shared_ptr<HestonProcess>& 
    HybridHestonHullWhiteProcess::hestonProcess() const {
        return hestonProcess_;
    }
    
    const ext::shared_ptr<HullWhiteForwardProcess>& 
    HybridHestonHullWhiteProcess::hullWhiteProcess() const {
        return hullWhiteProcess_;
    }

    HybridHestonHullWhiteProcess::Discretization 
    HybridHestonHullWhiteProcess::discretization() const {
        return discretization_;
    }
    
    Time HybridHestonHullWhiteProcess::time(const Date& date) const {
        return hestonProcess_->time(date);
    }

    void HybridHestonHullWhiteProcess::update() {
        endDiscount_ = hestonProcess_->riskFreeRate()->discount(T_);
    }
}
