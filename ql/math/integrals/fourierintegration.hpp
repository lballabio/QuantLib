/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file fourierintegration.hpp
    \brief integration of characteristic-function payoff transforms
*/

#ifndef quantlib_fourier_integration_hpp
#define quantlib_fourier_integration_hpp

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/integrals/integral.hpp>
#include <functional>

namespace QuantLib {

    //! Fourier integration used by characteristic-function pricing engines
    /*! Wraps a quadrature or an adaptive integrator together with the
        change of variables that maps the semi-infinite Fourier integration
        range onto the domain the chosen algorithm expects.  The decay rate
        \f$ c_\infty \f$ of the characteristic function along the
        integration contour drives that mapping.
    */
    class FourierIntegration {
      public:
        // non adaptive integration algorithms based on Gaussian quadrature
        static FourierIntegration gaussLaguerre    (Size integrationOrder = 128);
        static FourierIntegration gaussLegendre    (Size integrationOrder = 128);
        static FourierIntegration gaussChebyshev   (Size integrationOrder = 128);
        static FourierIntegration gaussChebyshev2nd(Size integrationOrder = 128);

        // Gatheral's version has to be used for an adaptive integration
        // algorithm .Be aware: using a too large number for maxEvaluations might
        // result in a stack overflow as the these integrations are based on
        // recursive algorithms.
        static FourierIntegration gaussLobatto(Real relTolerance, Real absTolerance,
                                               Size maxEvaluations = 1000,
                                               bool useConvergenceEstimate = false);

        // usually these routines have a poor convergence behavior.
        static FourierIntegration gaussKronrod(Real absTolerance,
                                               Size maxEvaluations = 1000);
        static FourierIntegration simpson(Real absTolerance,
                                          Size maxEvaluations = 1000);
        static FourierIntegration trapezoid(Real absTolerance,
                                            Size maxEvaluations = 1000);
        static FourierIntegration discreteSimpson(Size evaluation = 1000);
        static FourierIntegration discreteTrapezoid(Size evaluation = 1000);
        static FourierIntegration expSinh(Real relTolerance = 1e-8);

        static Real andersenPiterbargIntegrationLimit(
            Real c_inf, Real epsilon, Real v0, Real t);

        Real calculate(Real c_inf,
                       const std::function<Real(Real)>& f,
                       const std::function<Real()>& maxBound = {},
                       Real scaling = 1.0) const;

        Real calculate(Real c_inf,
                       const std::function<Real(Real)>& f,
                       Real maxBound) const;

        Size numberOfEvaluations() const;
        bool isAdaptiveIntegration() const;

      private:
        enum Algorithm
            { GaussLobatto, GaussKronrod, Simpson, Trapezoid,
              DiscreteTrapezoid, DiscreteSimpson,
              GaussLaguerre, GaussLegendre,
              GaussChebyshev, GaussChebyshev2nd,
              ExpSinh};

        FourierIntegration(Algorithm intAlgo, ext::shared_ptr<GaussianQuadrature> quadrature);

        FourierIntegration(Algorithm intAlgo, ext::shared_ptr<Integrator> integrator);

        const Algorithm intAlgo_;
        const ext::shared_ptr<Integrator> integrator_;
        const ext::shared_ptr<GaussianQuadrature> gaussianQuadrature_;
    };
}

#endif
