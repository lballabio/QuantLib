/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

/*! \file hybridhestonhullwhiteprocess.hpp
    \brief hybrid equity (heston model)
           with stochastic interest rates (hull white model)
*/

#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>

namespace QuantLib {

    namespace {

        static std::vector<boost::shared_ptr<StochasticProcess> >
        buildProcessList(
          const boost::shared_ptr<HestonProcess> & hestonProcess,
          const boost::shared_ptr<HullWhiteForwardProcess> & hullWhiteProcess)
        {
            std::vector<boost::shared_ptr<StochasticProcess> > retVal;

            retVal.push_back(hestonProcess);
            retVal.push_back(hullWhiteProcess);

            return retVal;
        }

    }

    HybridHestonHullWhiteProcess::HybridHestonHullWhiteProcess(
        const boost::shared_ptr<HestonProcess> & hestonProcess,
        const boost::shared_ptr<HullWhiteForwardProcess> & hullWhiteProcess,
        Real corrEquityShortRate,
        Size factors)
    : JointStochasticProcess(buildProcessList(hestonProcess, hullWhiteProcess),
                             factors),
      hullWhiteModel_(new HullWhite(hestonProcess->riskFreeRate(),
                                    hullWhiteProcess->a(),
                                    hullWhiteProcess->sigma())),
      corrEquityShortRate_(corrEquityShortRate),

      T_(hullWhiteProcess->getForwardMeasureTime()),
      endDiscount_(hestonProcess->riskFreeRate()->discount(T_)) {

        QL_REQUIRE(  corrEquityShortRate*corrEquityShortRate
                    +hestonProcess->rho()*hestonProcess->rho() <= 1.0,
                    "correlation matrix has negative eigenvalues");
    }


    void HybridHestonHullWhiteProcess::preEvolve(Time t0, const Array& x0,
                                                 Time dt, const Array& dw)
    const { }

    Disposable<Array>
    HybridHestonHullWhiteProcess::postEvolve(Time t0, const Array& x0,
                                             Time dt, const Array& dw,
                                             const Array& y0) const {
        // recalculate equity process to reduce discretization
        // errors within the pure heston equity process
        Array retVal(y0);
        
        const boost::shared_ptr<HestonProcess> 
                                         hestonProcess(this->hestonProcess());
        const boost::shared_ptr<HullWhiteForwardProcess> 
                                   hullWhiteProcess(this->hullWhiteProcess());
        
        const Rate r         = x0[2];
        const Real a         = hullWhiteProcess->a();
        const Real sigma     = hullWhiteProcess->sigma();
        const Real rho       = corrEquityShortRate_;
        const Volatility eta = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;

        const Time s = t0;
        const Time t = t0 + dt;
        const Time T = T_;
        const Rate dy
            = hestonProcess->dividendYield()->forwardRate(s, t, Continuous,
                                                          NoFrequency);

        const Real df
            = std::log(  hestonProcess->riskFreeRate()->discount(t)
                       / hestonProcess->riskFreeRate()->discount(s));

        const Real eaT=std::exp(-a*T);
        const Real eat=std::exp(-a*t);
        const Real eas=std::exp(-a*s);
        const Real iat=1.0/eat;
        const Real ias=1.0/eas;

        const Real m1 = -(dy+0.5*eta*eta)*dt - df;

        const Real m2 = -rho*sigma*eta/a*(dt-1/a*eaT*(iat-ias));

        const Real m3 = (r - hullWhiteProcess->alpha(s))
                       *hullWhiteProcess->B(s,t);

        const Real m4 = sigma*sigma/(2*a*a)
            *(dt + 2/a*(eat-eas) - 1/(2*a)*(eat*eat-eas*eas));

        const Real m5 = -sigma*sigma/(a*a)
            *(dt - 1/a*(1-eat*ias) - 1/(2*a)*eaT*(iat-2*ias+eat*ias*ias));

        const Real v1 = sigma*sigma/(a*a)
            *(dt - 2/a*(1-eat*ias)
              + 1/(2*a)*(1-eat*eat*ias*ias));
        const Real v2 = eta*eta*dt;

        // todo: better discretization here
        const Real vol = std::sqrt(v1)*dw[2] + std::sqrt(v2)*dw[0];
        const Real mu = m1 + m2 + m3 + m4 + m5;
        retVal[0] = x0[0]*std::exp(mu + vol);

        return retVal;
    }

    Disposable<Matrix>
    HybridHestonHullWhiteProcess::crossModelCorrelation(
        Time t0, const Array& x0) const {

        const Size size = 3;
        Matrix retVal(size, size, 0.0);

        retVal[0][2] = retVal[2][0] = corrEquityShortRate_;

        return retVal;
    }

    bool HybridHestonHullWhiteProcess::correlationIsStateDependent() const {
        return false;
    }

    boost::shared_ptr<HestonProcess> 
    HybridHestonHullWhiteProcess::hestonProcess() const {
        return boost::dynamic_pointer_cast<HestonProcess>(l_[0]);
    }

    boost::shared_ptr<HullWhiteForwardProcess> 
    HybridHestonHullWhiteProcess::hullWhiteProcess() const {
        return boost::dynamic_pointer_cast<HullWhiteForwardProcess>(l_[1]);
    }

    DiscountFactor
    HybridHestonHullWhiteProcess::numeraire(Time t, const Array& x) const {

        return hullWhiteModel_->discountBond(t, T_, x[2]) / endDiscount_;
    }

    Real HybridHestonHullWhiteProcess::correlation() const {
        return corrEquityShortRate_;
    }

    void HybridHestonHullWhiteProcess::update() {
        const boost::shared_ptr<HestonProcess> hestonProcess(
                         boost::dynamic_pointer_cast<HestonProcess>(l_[0]));

        endDiscount_ = hestonProcess->riskFreeRate()->discount(T_);

        this->JointStochasticProcess::update();
    }
}

