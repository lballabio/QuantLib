/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb

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

/*! \file gammadistribution.hpp
    \brief Gamma distribution
*/

#ifndef quantlib_math_gamma_distribution_h
#define quantlib_math_gamma_distribution_h

#include <ql/errors.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    class CumulativeGammaDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        CumulativeGammaDistribution(Real a) : a_(a) {
            QL_REQUIRE(a>0.0, "invalid parameter for gamma distribution");
        }
        Real operator()(Real x) const;
      private:
        Real a_;
    };

    //! Gamma function class
    /*! This is a function defined by
        \f[
            \Gamma(z) = \int_0^{\infty}t^{z-1}e^{-t}dt
        \f]

        The implementation of the algorithm was inspired by
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6

        \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class GammaFunction {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        static Real value(Real x);
        static Real logValue(Real x);
    };

}


#endif
