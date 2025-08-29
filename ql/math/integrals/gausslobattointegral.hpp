/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file gausslobattointegral.hpp
    \brief integral of a one-dimensional function using the adaptive
    Gauss-Lobatto integral
*/

#ifndef quantlib_gauss_lobatto_integral_hpp
#define quantlib_gauss_lobatto_integral_hpp

#include <ql/errors.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/integrals/integral.hpp>

namespace QuantLib {

    //! Integral of a one-dimensional function
    /*! Given a target accuracy \f$ \epsilon \f$, the integral of
        a function \f$ f \f$ between \f$ a \f$ and \f$ b \f$ is
        calculated by means of the Gauss-Lobatto formula
    */

    /*! References:
       This algorithm is a C++ implementation of the algorithm outlined in

       W. Gander and W. Gautschi, Adaptive Quadrature - Revisited.
       BIT, 40(1):84-101, March 2000. CS technical report:
       ftp.inf.ethz.ch/pub/publications/tech-reports/3xx/306.ps.gz

       The original MATLAB version can be downloaded here
       http://www.inf.ethz.ch/personal/gander/adaptlob.m
    */

    class GaussLobattoIntegral : public Integrator {
      public:
        GaussLobattoIntegral(Size maxIterations,
                             Real absAccuracy,
                             Real relAccuracy = Null<Real>(),
                             bool useConvergenceEstimate = true);

      protected:
        Real integrate(const std::function<Real(Real)>& f, Real a, Real b) const override;

        Real adaptivGaussLobattoStep(const std::function<Real (Real)>& f,
                                     Real a, Real b, Real fa, Real fb,
                                     Real is) const;
        Real calculateAbsTolerance(const std::function<Real (Real)>& f,
                                   Real a, Real b) const;

        Real relAccuracy_;
        const bool useConvergenceEstimate_;
        const static Real alpha_, beta_, x1_, x2_, x3_;
    };
}
#endif
