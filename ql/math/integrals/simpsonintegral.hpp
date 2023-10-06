/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Roman Gitlin
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file simpsonintegral.hpp
    \brief integral of a one-dimensional function using Simpson formula
*/

#ifndef quantlib_simpson_integral_hpp
#define quantlib_simpson_integral_hpp

#include <ql/math/integrals/trapezoidintegral.hpp>

namespace QuantLib {

    //! Integral of a one-dimensional function
    /*! \test the correctness of the result is tested by checking it
              against known good values.
    */
    class SimpsonIntegral : public TrapezoidIntegral<Default> {
      public:
        SimpsonIntegral(Real accuracy,
                        Size maxIterations)
        : TrapezoidIntegral<Default>(accuracy, maxIterations) {}
      protected:
        Real integrate(const ext::function<Real(Real)>& f, Real a, Real b) const override {

            // start from the coarsest trapezoid...
            Size N = 1;
            Real I = (f(a)+f(b))*(b-a)/2.0, newI;
            increaseNumberOfEvaluations(2);

            Real adjI = I, newAdjI;
            // ...and refine it
            Size i = 1;
            do {
                newI = Default::integrate(f,a,b,I,N);
                increaseNumberOfEvaluations(N);
                N *= 2;
                newAdjI = (4.0*newI-I)/3.0;
                // good enough? Also, don't run away immediately
                if (std::fabs(adjI-newAdjI) <= absoluteAccuracy() && i > 5)
                    // ok, exit
                    return newAdjI;
                // oh well. Another step.
                I = newI;
                adjI = newAdjI;
                i++;
            } while (i < maxEvaluations());
            QL_FAIL("max number of iterations reached");
        }
    };

}

#endif
