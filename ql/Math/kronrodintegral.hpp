
/*
 Copyright (C) 2003 Niels Elken Sønderby

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file kronrodintegral.hpp
    \brief Integral of a 1-dimensional function using the Gauss-Kronrod method

*/

#ifndef quantlib_kronrod_integral_h
#define quantlib_kronrod_integral_h

#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

namespace Math {

    //! Integral of a 1-dimensional function using the Gauss-Kronrod method
    /*! References:  Gauss-Kronrod Integration
        http://mathcssun1.emporia.edu/~oneilcat/ExperimentApplet3/ExperimentApplet3.html
        NMS - Numerical Analysis Library
        <http://www.math.iastate.edu/burkardt/f_src/nms/nms.html>
    */

    class KronrodIntegral {
      public:
        KronrodIntegral(double tolerance_, int maxRecursionDepth);

        template <class F>
        double operator()(const F& f, double a, double b) {
            QL_REQUIRE(a < b,
                       "to compute an integral on [a,b] it must be a<b; "
                       "a="+DoubleFormatter::toString(a)+", "
                       "b="+DoubleFormatter::toString(b));
            functionEvaluations_ = 0;

            return GaussKronrod(f, a, b, tolerance_, 0);
        }

        int functionEvaluations() { return functionEvaluations_; }

      private:

        template <class F>
        double GaussKronrod(const F& f,
                            const double a, const double b,
                            const double tolerance, int recursionDepth) {
            // weights for 7-point Gauss-Legendre integration
            // (only 4 values out of 7 are given as they are symmetric)
            static const double g7w[]= { 0.417959183673469,
                0.381830050505119, 0.279705391489277, 0.129484966168870 };
            // weights for 15-point Gauss-Kronrod integration
            static const double k15w[]={ 0.209482141084728, 0.204432940075298,
                                  0.190350578064785, 0.169004726639267,
                                  0.140653259715525, 0.104790010322250,
                                  0.063092092629979, 0.022935322010529 };
            // abscissae (evaluation points)
            // for 15-point Gauss-Kronrod integration
            static const double k15t[]={ 0.000000000000000, 0.207784955007898,
                                  0.405845151377397, 0.586087235467691,
                                  0.741531185599394, 0.864864423359769,
                                  0.949107912342758, 0.991455371120813 };

            const double halflength = (b - a) / 2;
            const double center = (a + b) / 2;

            double g7; // will be result of G7 integral
            double k15; // will be result of K15 integral

            double t, fsum; // t (abscissa) and f(t)
            double fc = f(center);
            g7 = fc * g7w[0];
            k15 = fc * k15w[0];

            // calculate g7 and half of k15
            int j, j2;
            for (j = 1, j2 = 2; j < 4; j++, j2 += 2) {
                t = halflength * k15t[j2];
                fsum = f(center - t) + f(center + t);
                g7  += fsum * g7w[j];
                k15 += fsum * k15w[j2];
            }

            // calculate other half of k15
            for (j2 = 1; j2 < 8; j2 += 2) {
                t = halflength * k15t[j2];
                fsum = f(center - t) + f(center + t);
                k15 += fsum * k15w[j2];
            }

            // multiply by (a - b) / 2
            g7 = halflength * g7;
            k15 = halflength * k15;

            // 15 more function evaluations has been used
            functionEvaluations_ += 15;

            // error is <= k15 - g7
            // if error is larger than tolerance then split in interval
            // in two and integrate recursively
            if (QL_FABS(k15 - g7) < tolerance)
                return k15;
            else {
                  QL_REQUIRE (recursionDepth++ <= maxRecursionDepth_,
                    "maxRecursionDepth of "
                    + IntegerFormatter::toString(maxRecursionDepth_)
                    + " exceeded");
                return GaussKronrod(f, a, center,
                           tolerance/2, recursionDepth + 1)
                       + GaussKronrod(f, center, b,
                           tolerance/2, recursionDepth + 1);
            }
        }

        double tolerance_;
        int maxRecursionDepth_;
        int functionEvaluations_;
    };

     inline KronrodIntegral::KronrodIntegral
         (double tolerance, int maxRecursionDepth = 50)
    : tolerance_(tolerance), maxRecursionDepth_(maxRecursionDepth) {
        QL_REQUIRE(tolerance > QL_EPSILON, "tolerance must be > 0");
    }

}

}

#endif
