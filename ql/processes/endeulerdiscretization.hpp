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

/*! \file endeulerdiscretization.hpp
    \brief Euler end-point discretization for stochastic processes
*/

#ifndef quantlib_end_euler_discretization_hpp
#define quantlib_end_euler_discretization_hpp

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! Euler end-point discretization for stochastic processes
    /*! \ingroup processes */
    class EndEulerDiscretization
        : public StochasticProcess::discretization,
          public StochasticProcess1D::discretization {
      public:

        /*! Returns an approximation of the drift defined as
            \f$ \mu(t_0 + \Delta t, \mathbf{x}_0) \Delta t \f$.
        */
        Disposable<Array>
        drift(const StochasticProcess&, Time t0, const Array& x0, Time dt) const override;
        /*! Returns an approximation of the drift defined as
            \f$ \mu(t_0 + \Delta t, x_0) \Delta t \f$.
        */
        Real drift(const StochasticProcess1D&, Time t0, Real x0, Time dt) const override;

        /*! Returns an approximation of the diffusion defined as
            \f$ \sigma(t_0 + \Delta t, \mathbf{x}_0) \sqrt{\Delta t} \f$.
        */
        Disposable<Matrix>
        diffusion(const StochasticProcess&, Time t0, const Array& x0, Time dt) const override;
        /*! Returns an approximation of the diffusion defined as
            \f$ \sigma(t_0 + \Delta t, x_0) \sqrt{\Delta t} \f$.
        */
        Real diffusion(const StochasticProcess1D&, Time t0, Real x0, Time dt) const override;

        /*! Returns an approximation of the covariance defined as
            \f$ \sigma(t_0 + \Delta t, \mathbf{x}_0)^2 \Delta t \f$.
        */
        Disposable<Matrix>
        covariance(const StochasticProcess&, Time t0, const Array& x0, Time dt) const override;
        /*! Returns an approximation of the variance defined as
            \f$ \sigma(t_0 + \Delta t, x_0)^2 \Delta t \f$.
        */
        Real variance(const StochasticProcess1D&, Time t0, Real x0, Time dt) const override;
    };

}


#endif

