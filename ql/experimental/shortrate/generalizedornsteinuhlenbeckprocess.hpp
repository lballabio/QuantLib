/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 SunTrust Bank
 Copyright (C) 2010 Cavit Hafizoglu

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

/*! \file generalizedornsteinuhlenbeckprocess.hpp
    \brief Ornstein-Uhlenbeck process with piecewise linear coefficients
*/

#ifndef quantlib_generalized_ornstein_uhlenbeck_process_hpp
#define quantlib_generalized_ornstein_uhlenbeck_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    //! Piecewise linear Ornstein-Uhlenbeck process class
    /*! This class describes the Ornstein-Uhlenbeck process governed by
        \f[
            dx = a (level - x_t) dt + \sigma dW_t
        \f]

        \ingroup processes

        where the coefficients a and sigma are piecewise linear.
    */
    class GeneralizedOrnsteinUhlenbeckProcess : public StochasticProcess1D {
      public:
        GeneralizedOrnsteinUhlenbeckProcess(std::function<Real(Time)> speed,
                                            std::function<Real(Time)> vol,
                                            Real x0 = 0.0,
                                            Real level = 0.0);
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const override;

        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;

        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;
        //@}

        Real speed(Time t) const;
        Real volatility(Time t) const;
        Real level() const;

      private:
        Real x0_, level_;
        std::function<Real (Time)> speed_;
        std::function<Real (Time)> volatility_;
    };

}


#endif
