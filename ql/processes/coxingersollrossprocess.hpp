/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao
 Copyright (C) 2021 Magnus Mencke

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

/*! \file coxingersollross.hpp
    \brief CoxIngersollRoss process
*/

#ifndef quantlib_coxingersollross_process_hpp
#define quantlib_coxingersollross_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! CoxIngersollRoss process class
    /*! This class describes the CoxIngersollRoss process governed by
        \f[
            dx(t) = k (\theta - x(t)) dt + \sigma \sqrt{x(t)} dW(t).
        \f]

        The process is discretized using the Quadratic Exponential scheme.
        For details see Leif Andersen,
        Efficient Simulation of the Heston Stochastic Volatility Model.

        \ingroup processes
    */
    class CoxIngersollRossProcess : public StochasticProcess1D {
      public:

        CoxIngersollRossProcess(Real speed,
                                 Volatility vol,
                                 Real x0 = 0.0,
                                 Real level = 0.0);
        //@{
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        //@}
        Real x0() const override;
        Real speed() const;
        Real volatility() const;
        Real level() const;
        Real variance(Time t0, Real x0, Time dt) const override;
        Real evolve (Time t0,
                     Real x0,
                     Time dt,
                     Real dw) const override;
      private:
        Real x0_, speed_, level_;
        Volatility volatility_;
    };

    // inline

    inline Real CoxIngersollRossProcess::x0() const {
        return x0_;
    }

    inline Real CoxIngersollRossProcess::speed() const {
        return speed_;
    }

    inline Real CoxIngersollRossProcess::volatility() const {
        return volatility_;
    }

    inline Real CoxIngersollRossProcess::level() const {
        return level_;
    }

    inline Real CoxIngersollRossProcess::drift(Time, Real x) const {
        return speed_ * (level_ - x);
    }

    inline Real CoxIngersollRossProcess::diffusion(Time, Real) const {
        return volatility_;
    }

    inline Real CoxIngersollRossProcess::expectation(Time, Real x0,
                                               Time dt) const {
        return level_ + (x0 - level_) * std::exp(-speed_*dt);
    }

    inline Real CoxIngersollRossProcess::stdDeviation(Time t, Real x0,
                                                Time dt) const {
        return std::sqrt(variance(t,x0,dt));
    }

    inline Real CoxIngersollRossProcess::evolve (Time t0,
      Real x0,
                                    Time dt,
                                    Real dw) const {
        Real result;

        const Real ex = std::exp(-speed_*dt);

        const Real m  =  level_+(x0-level_)*ex;
        const Real s2 =  x0*volatility_*volatility_*ex/speed_*(1-ex)
                       + level_*volatility_*volatility_/(2*speed_)*(1-ex)*(1-ex);
        const Real psi = s2/(m*m);

        if (psi <= 1.5) {
            const Real b2 = 2/psi-1+std::sqrt(2/psi*(2/psi-1));
            const Real b  = std::sqrt(b2);
            const Real a  = m/(1+b2);

            result = a*(b+dw)*(b+dw);
        }
        else {
            const Real p = (psi-1)/(psi+1);
            const Real beta = (1-p)/m;

            const Real u = CumulativeNormalDistribution()(dw);

            result = ((u <= p) ? 0.0 : std::log((1-p)/(1-u))/beta);
        }

        return result;
    }

}

#endif


#ifndef id_478c84e1d935c6589443e3d2b50447cb
#define id_478c84e1d935c6589443e3d2b50447cb
inline bool test_478c84e1d935c6589443e3d2b50447cb(int* i) { return i != 0; }
#endif
