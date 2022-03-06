/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2003 Niels Elken Sønderby

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

/*! \file kronrodintegral.hpp
    \brief Integral of a 1-dimensional function using the Gauss-Kronrod method
*/

#ifndef quantlib_kronrod_integral_hpp
#define quantlib_kronrod_integral_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    //! Integral of a 1-dimensional function using the Gauss-Kronrod methods
    /*! This class provide a non-adaptive integration procedure which
        uses fixed Gauss-Kronrod abscissae to sample the integrand at
        a maximum of 87 points.  It is provided for fast integration
        of smooth functions.

        This function applies the Gauss-Kronrod 10-point, 21-point, 43-point
        and 87-point integration rules in succession until an estimate of the
        integral of f over (a, b) is achieved within the desired absolute and
        relative error limits, epsabs and epsrel. The function returns the
        final approximation, result, an estimate of the absolute error,
        abserr and the number of function evaluations used, neval. The
        Gauss-Kronrod rules are designed in such a way that each rule uses
        all the results of its predecessors, in order to minimize the total
        number of function evaluations.
    */
    class GaussKronrodNonAdaptive : public Integrator {
      public:
        GaussKronrodNonAdaptive(Real absoluteAccuracy,
                                Size maxEvaluations,
                                Real relativeAccuracy);
        void setRelativeAccuracy(Real);
        Real relativeAccuracy() const;
      protected:
        Real integrate(const ext::function<Real(Real)>& f, Real a, Real b) const override;

      private:
        Real relativeAccuracy_;
    };

    //! Integral of a 1-dimensional function using the Gauss-Kronrod methods
    /*! This class provide an adaptive integration procedure using 15
        points Gauss-Kronrod integration rule.  This is more robust in
        that it allows to integrate less smooth functions (though
        singular functions should be integrated using dedicated
        algorithms) but less efficient beacuse it does not reuse
        precedently computed points during computation steps.

        References:

        Gauss-Kronrod Integration
        <http://mathcssun1.emporia.edu/~oneilcat/ExperimentApplet3/ExperimentApplet3.html>

        NMS - Numerical Analysis Library
        <http://www.math.iastate.edu/burkardt/f_src/nms/nms.html>

        \test the correctness of the result is tested by checking it
              against known good values.
    */
    class GaussKronrodAdaptive : public Integrator {
      public:
        explicit GaussKronrodAdaptive(Real tolerance,
                                      Size maxFunctionEvaluations = Null<Size>());
      protected:
        Real integrate(const ext::function<Real(Real)>& f, Real a, Real b) const override;

      private:
          Real integrateRecursively(const ext::function<Real (Real)>& f,
                                    Real a,
                                    Real b,
                                    Real tolerance) const;
      };
}

#endif


#ifndef id_688290f1bcaca9cd15410ad6cfca39da
#define id_688290f1bcaca9cd15410ad6cfca39da
inline bool test_688290f1bcaca9cd15410ad6cfca39da(int* i) { return i != 0; }
#endif
