/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

namespace QuantLib {

    //! CoxIngersollRoss process class
    /*! This class describes the CoxIngersollRoss process governed by
        \f[
            dx = a (r - x_t) dt + \sqrt{x_t}\sigma dW_t.
        \f]

        \ingroup processes
    */
    class CoxIngersollRossProcess : public StochasticProcess1D {
      public:
        CoxIngersollRossProcess(Real speed,
                                 Volatility vol,
                                 Real x0 = 0.0,
                                 Real level = 0.0);
        //@{
        Real drift(Time t,
                   Real x) const;
        Real diffusion(Time t,
                       Real x) const;
        Real expectation(Time t0,
                         Real x0,
                         Time dt) const;
        Real stdDeviation(Time t0,
                          Real x0,
                          Time dt) const;
        //@}
        Real x0() const;
        Real speed() const;
        Real volatility() const;
        Real level() const;
        Real variance(Time t0,
                      Real x0,
                      Time dt) const;
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

}

#endif
