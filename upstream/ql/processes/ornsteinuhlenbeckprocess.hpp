/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file ornsteinuhlenbeckprocess.hpp
    \brief Ornstein-Uhlenbeck process
*/

#ifndef quantlib_ornstein_uhlenbeck_process_hpp
#define quantlib_ornstein_uhlenbeck_process_hpp

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! Ornstein-Uhlenbeck process class
    /*! This class describes the Ornstein-Uhlenbeck process governed by
        \f[
            dx = a (r - x_t) dt + \sigma dW_t.
        \f]

        \ingroup processes
    */
    class OrnsteinUhlenbeckProcess : public StochasticProcess1D {
      public:
        OrnsteinUhlenbeckProcess(Real speed,
                                 Volatility vol,
                                 Real x0 = 0.0,
                                 Real level = 0.0);
        //! \name StochasticProcess interface
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

      private:
        Real x0_, speed_, level_;
        Volatility volatility_;
    };

    // inline

    inline Real OrnsteinUhlenbeckProcess::x0() const {
        return x0_;
    }

    inline Real OrnsteinUhlenbeckProcess::speed() const {
        return speed_;
    }

    inline Real OrnsteinUhlenbeckProcess::volatility() const {
        return volatility_;
    }

    inline Real OrnsteinUhlenbeckProcess::level() const {
        return level_;
    }

    inline Real OrnsteinUhlenbeckProcess::drift(Time, Real x) const {
        return speed_ * (level_ - x);
    }

    inline Real OrnsteinUhlenbeckProcess::diffusion(Time, Real) const {
        return volatility_;
    }

    inline Real OrnsteinUhlenbeckProcess::expectation(Time, Real x0,
                                               Time dt) const {
        return level_ + (x0 - level_) * std::exp(-speed_*dt);
    }

    inline Real OrnsteinUhlenbeckProcess::stdDeviation(Time t, Real x0,
                                                Time dt) const {
        return std::sqrt(variance(t,x0,dt));
    }

}

#endif
