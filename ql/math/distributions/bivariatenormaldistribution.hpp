/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2005 Gary Kennedy

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

/*! \file bivariatenormaldistribution.hpp
    \brief bivariate cumulative normal distribution
*/

#ifndef quantlib_bivariatenormal_distribution_hpp
#define quantlib_bivariatenormal_distribution_hpp

#include <ql/math/distributions/normaldistribution.hpp>

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
                 Journal of Statistical Computation and Simulation 35,
                 pp. 101-107.
              3) Drezner, Z (1992)
                 Computation of the Multivariate Normal Integral,
                 ACM Transactions on Mathematics Software 18, pp. 450-460.
              4) Drezner, Z (1994)
                 Computation of the Trivariate Normal Integral,
                 Mathematics of Computation 62, pp. 289-294.
              5) Genz, A. (1992)
                `Numerical Computation of the Multivariate Normal
                 Probabilities', J. Comput. Graph. Stat. 1, pp. 141-150.

        \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class BivariateCumulativeNormalDistributionDr78 {
      public:
        BivariateCumulativeNormalDistributionDr78(Real rho);
        // function
        Real operator()(Real a, Real b) const;
      private:
        Real rho_, rho2_;
        static const Real x_[], y_[];
    };


    //! Cumulative bivariate normal distibution function (West 2004)
    /*! The implementation derives from the article "Better
        Approximations To Cumulative Normal Distibutions", Graeme
        West, Dec 2004 available at www.finmod.co.za. Also available
        in Wilmott Magazine, 2005, (May), 70-76, The main code is a
        port of the C++ code at www.finmod.co.za/cumfunctions.zip.

        The algorithm is based on the near double-precision algorithm
        described in "Numerical Computation of Rectangular Bivariate
        an Trivariate Normal and t Probabilities", Genz (2004),
        Statistics and Computing 14, 151-160. (available at
        www.sci.wsu.edu/math/faculty/henz/homepage)

        The QuantLib implementation mainly differs from the original
        code in two regards;
        - The implementation of the cumulative normal distribution is
          QuantLib::CumulativeNormalDistribution
        - The arrays XX and W are zero-based

        \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class BivariateCumulativeNormalDistributionWe04DP {
      public:
        BivariateCumulativeNormalDistributionWe04DP(Real rho);
        // function
        Real operator()(Real a, Real b) const;
      private:
        Real correlation_;
        CumulativeNormalDistribution cumnorm_;
    };

    //! default bivariate implementation
    typedef BivariateCumulativeNormalDistributionWe04DP
                                        BivariateCumulativeNormalDistribution;

}


#endif


#ifndef id_2e7a29d2b58742f3a7368f9d14dafa3d
#define id_2e7a29d2b58742f3a7368f9d14dafa3d
inline bool test_2e7a29d2b58742f3a7368f9d14dafa3d(const int* i) {
    return i != nullptr;
}
#endif
