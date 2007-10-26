/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

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

            // copy of the risk free term structure for the control variate
            Handle<YieldTermStructure> rf(*(hestonProcess->riskFreeRate()));
            retVal.push_back(boost::shared_ptr<HestonProcess>(
                new HestonProcess(rf,
                                  hestonProcess->dividendYield(),
                                  hestonProcess->s0(),
                                  hestonProcess->v0()->value(),
                                  hestonProcess->kappa()->value(),
                                  hestonProcess->theta()->value(),
                                  hestonProcess->sigma()->value(),
                                  hestonProcess->rho()->value())));
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
    }


    void HybridHestonHullWhiteProcess::preEvolve(Time t0, const Array& x0,
                                                 Time dt, const Array& dw)
    const { }

    Disposable<Array>
    HybridHestonHullWhiteProcess::postEvolve(Time t0, const Array& x0,
                                             Time dt, const Array& dw,
                                             const Array& y0) const {
        Array dv(2), x(2);

        // random number from the original heston process
        dv[0] = dw[0]; dv[1] = dw[1];

        // state from the control variate process
        x[0] = x0[2]; x[1] = x0[3];

        // control variate process
        boost::shared_ptr<HestonProcess> hestonProcess
            = boost::dynamic_pointer_cast<HestonProcess>(l_[1]);

        // evolve the control variate
        const Array y = hestonProcess->evolve(t0, x, dt, dv);

        // write back results to the full hybrid process
        Array retVal(y0);
        retVal[2] = y[0]; retVal[3] = y[1];

        boost::shared_ptr<HullWhiteForwardProcess> hullWhiteProcess
            = boost::dynamic_pointer_cast<HullWhiteForwardProcess>(l_[2]);

        // recalculate equity process to reduce discretization
        // errors within the pure heston equity process
        const Rate r         = x0[4];
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

        const Real m1 = -(dy+0.5*eta*eta)*dt - df;

        const Real m2 = -rho*sigma*eta/a*(dt-1/a*(  std::exp(-a*(T-t))
                                                  - std::exp(-a*(T-s))));

        const Real m3 = (r - hullWhiteProcess->alpha(s))
                       *hullWhiteProcess->B(s,t);

        const Real m4 = sigma*sigma/(2*a*a)
            *(dt + 2/a*(std::exp(-a*t)-std::exp(-a*s))
              - 1/(2*a)*(std::exp(-2*a*t)-std::exp(-2*a*s)));

        const Real m5 = -sigma*sigma/(a*a)
            *(dt - 1/a*(1-std::exp(-a*(t-s)))
                 - 1/(2*a)*(  std::exp(-a*(T-t))
                            - 2*std::exp(-a*(T-s)) + std::exp(-a*(T+t-2*s))));

        const Real mu = m1 + m2 + m3 + m4 + m5;

        const Real v1 = sigma*sigma/(a*a)
            *(dt - 2/a*(1-std::exp(-a*(t-s)))
              + 1/(2*a)*(1-std::exp(-2*a*(t-s))));

        const Real v2 = eta*eta*(t-s);

        const Real vol = std::sqrt(v1)*dw[4] + std::sqrt(v2)*dw[0];

        retVal[0] = x0[0]*std::exp(mu + vol);

        return retVal;
    }

    Disposable<Matrix>
    HybridHestonHullWhiteProcess::crossModelCorrelation(
        Time t0, const Array& x0) const {

        const Size size = 5;
        Matrix retVal(size, size, 0.0);

        retVal[0][4] = retVal[4][0] = corrEquityShortRate_;

        return retVal;
    }

    bool HybridHestonHullWhiteProcess::correlationIsStateDependend() const {
        return false;
    }

    DiscountFactor
    HybridHestonHullWhiteProcess::numeraire(Time t, const Array& x) const {

        return hullWhiteModel_->discountBond(t, T_, x[4]) / endDiscount_;
    }

    void HybridHestonHullWhiteProcess::update() {
        boost::shared_ptr<HestonProcess> hestonProcess
            = boost::dynamic_pointer_cast<HestonProcess>(l_[0]);

        boost::shared_ptr<HestonProcess> cvProcess
            = boost::dynamic_pointer_cast<HestonProcess>(l_[1]);

        l_[1] = boost::shared_ptr<HestonProcess>(
            new HestonProcess(cvProcess->riskFreeRate(),
                              hestonProcess->dividendYield(),
                              hestonProcess->s0(),
                              hestonProcess->v0()->value(),
                              hestonProcess->kappa()->value(),
                              hestonProcess->theta()->value(),
                              hestonProcess->sigma()->value(),
                              hestonProcess->rho()->value()));

        endDiscount_ = hestonProcess->riskFreeRate()->discount(T_);

        this->JointStochasticProcess::update();
    }
}


