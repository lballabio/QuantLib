/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Frank Hövermann

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

#include <ql/experimental/processes/extendedblackscholesprocess.hpp>

namespace QuantLib {

    ExtendedBlackScholesMertonProcess::ExtendedBlackScholesMertonProcess(
                              const Handle<Quote>& x0,
                              const Handle<YieldTermStructure>& dividendTS,
                              const Handle<YieldTermStructure>& riskFreeTS,
                              const Handle<BlackVolTermStructure>& blackVolTS,
                              const ext::shared_ptr<discretization>& d,
                              Discretization evolDisc)
    : GeneralizedBlackScholesProcess(x0,dividendTS,riskFreeTS,blackVolTS,d),
      discretization_(evolDisc) {}

    Real ExtendedBlackScholesMertonProcess::drift(Time t, Real x) const {
        Real sigma = diffusion(t,x);
        // we could be more anticipatory if we know the right dt
        // for which the drift will be used
        Time t1 = t + 0.0001;
        return (riskFreeRate()->forwardRate(t,t1,Continuous,NoFrequency,true)
             - dividendYield()->forwardRate(t,t1,Continuous,NoFrequency,true) ).rate()
             - 0.5 * sigma * sigma;
    }

    Real ExtendedBlackScholesMertonProcess::diffusion(Time t, Real x) const {
        return blackVolatility()->blackVol(t, x, true);
    }

    Real ExtendedBlackScholesMertonProcess::evolve(Time t0, Real x0,
                                                   Time dt, Real dw) const {
        Real predictor, sigma0, sigma1;
        Time t1;
        Rate rate0, rate1;
        Real driftterm, diffusionterm, corrector;
        switch (discretization_) {
          case Milstein:
            // Milstein scheme
            return apply(x0, drift(t0, x0)*dt
                           + 0.5*std::pow(diffusion(t0, x0),2)*(dw*dw-1)*dt
                           + diffusion(t0,x0)*std::sqrt(dt)*dw);
            break;
          case Euler:
            // Usual Euler scheme
            return apply(expectation(t0,x0,dt), stdDeviation(t0,x0,dt)*dw);
            break;
          case PredictorCorrector:
            // Predictor-Corrector scheme with equal weighting
            predictor =
                apply(expectation(t0,x0,dt), stdDeviation(t0,x0,dt)*dw);
            t1 = t0 + 0.0001;
            sigma0 = diffusion(t0,x0);
            sigma1 = diffusion(t0+dt,predictor);
            rate0 =
                (riskFreeRate()->forwardRate(t0,t1,Continuous,NoFrequency,true)
              - dividendYield()->forwardRate(t0,t1,Continuous,NoFrequency,true) ).rate()
              - 0.5*std::pow(sigma0,2);
            rate1 =
                (riskFreeRate()->forwardRate(t0+dt,t1+dt,Continuous,
                                            NoFrequency,true)
              - dividendYield()->forwardRate(t0+dt,t1+dt,
                                             Continuous,NoFrequency,true) ).rate()
              - 0.5*std::pow(sigma1,2);
            driftterm = 0.5*rate1+0.5*rate0;
            diffusionterm = 0.5*(sigma1+sigma0);
            corrector =
                apply(x0,driftterm*dt+diffusionterm*std::sqrt(dt)*dw);
            return corrector;
            break;
          default:
            QL_FAIL("unknown discretization scheme");
        }
    }

}
