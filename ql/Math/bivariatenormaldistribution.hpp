
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bivariatenormaldistribution.hpp
    \brief bivariate cumulative normal distribution
*/

#ifndef quantlib_bivariatenormal_distribution_h
#define quantlib_bivariatenormal_distribution_h

#include <ql/dataformatters.hpp>

namespace QuantLib {

    //! Cumulative bivariate normal distribution function
    /*! Drezner (1978) algorithm, six decimal places accuracy.

        For this implementation see
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998

        \todo check accuracy of this algorithm and compare with:
              1) Drezner, Z, (1978),
                 Computation of the bivariate normal integral,
                 Mathematics of Computation 32, pp. 277-279.
              2) Drezner, Z. and Wesolowsky, G. O. (1990)
                 `On the Computation of the Bivariate Normal Integral',
                 Journal of Statistical Computation and Simulation 35, pp. 101-107.
              3) Drezner, Z (1992)
                 Computation of the Multivariate Normal Integral,
                 ACM Transactions on Mathematics Software 18, pp. 450-460.
              4) Drezner, Z (1994)
                 Computation of the Trivariate Normal Integral,
                 Mathematics of Computation 62, pp. 289-294.
              5) Genz, A. (1992)
                `Numerical Computation of the Multivariate Normal Probabilities',
                 J. Comput. Graph. Stat. 1, pp. 141-150.

    */
    class BivariateCumulativeNormalDistribution {
      public:
        BivariateCumulativeNormalDistribution(double rho);
        // function
        double operator()(double a, double b) const;
      private:
        double rho_, rho2_;
        static const double x_[], y_[];
    };


    inline BivariateCumulativeNormalDistribution::BivariateCumulativeNormalDistribution(
                                                 double rho)
    : rho_(rho), rho2_(rho*rho) {

        QL_REQUIRE(rho>=-1.0,
                   "BivariateCumulativeNormalDistribution: "
                   "rho must be >= -1.0 (" +
                   DoubleFormatter::toString(rho) + " not allowed)");
        QL_REQUIRE(rho<=1.0,
                   "BivariateCumulativeNormalDistribution: "
                   "rho must be <= 1.0 (" +
                   DoubleFormatter::toString(rho) + " not allowed)");
    }

}


#endif
