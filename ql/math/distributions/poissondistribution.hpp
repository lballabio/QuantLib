/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2004 Walter Penschke

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

/*! \file poissondistribution.hpp
    \brief Poisson distribution
*/

#ifndef quantlib_poisson_distribution_hpp
#define quantlib_poisson_distribution_hpp

#include <ql/math/factorial.hpp>
#include <ql/math/incompletegamma.hpp>

namespace QuantLib {

    //! Poisson distribution function
    /*! Given an integer \f$ k \f$, it returns its probability
        in a Poisson distribution.

        \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class PoissonDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        PoissonDistribution(Real mu);
        // function
        Real operator()(BigNatural k) const;
      private:
        Real mu_, logMu_;
    };


    //! Cumulative Poisson distribution function
    /*! This function provides an approximation of the
        integral of the Poisson distribution.

        For this implementation see
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6

        \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class CumulativePoissonDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        CumulativePoissonDistribution(Real mu) : mu_(mu) {}
        Real operator()(BigNatural k) const {
            return 1.0 - incompleteGammaFunction(k+1, mu_);
        }
      private:
        Real mu_;
    };


    //! Inverse cumulative Poisson distribution function
    /*! \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class InverseCumulativePoisson {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        InverseCumulativePoisson(Real lambda = 1.0);
        Real operator()(Real x) const;
      private:
        Real lambda_;
        Real calcSummand(BigNatural index) const;
    };



    // inline definitions

    inline PoissonDistribution::PoissonDistribution(Real mu)
    : mu_(mu) {

        QL_REQUIRE(mu_>=0.0,
                   "mu must be non negative (" << mu_ << " not allowed)");

        if (mu_!=0.0) logMu_ = std::log(mu_);
    }

    inline Real PoissonDistribution::operator()(BigNatural k) const {
        if (mu_==0.0) {
            if (k==0) return 1.0;
            else      return 0.0;
        }
        Real logFactorial = Factorial::ln(k);
        return std::exp(k*std::log(mu_) - logFactorial - mu_);
    }


    inline InverseCumulativePoisson::InverseCumulativePoisson(Real lambda)
    : lambda_(lambda) {
        QL_REQUIRE(lambda_ > 0.0, "lambda must be positive");
    }

    inline Real InverseCumulativePoisson::operator()(Real x) const {
        QL_REQUIRE(x >= 0.0 && x <= 1.0,
                   "Inverse cumulative Poisson distribution is "
                   "only defined on the interval [0,1]");

        if (x == 1.0)
            return QL_MAX_REAL;

        Real sum = 0.0;
        BigNatural index = 0;
        while (x > sum) {
            sum += calcSummand(index);
            index++;
        }

        return Real(index-1);
    }

    inline Real InverseCumulativePoisson::calcSummand(BigNatural index) const {
        return std::exp(-lambda_) * std::pow(lambda_, Integer(index)) /
            Factorial::get(index);
    }

}


#endif


#ifndef id_c74847a08a08875d258a0e055b1f2d23
#define id_c74847a08a08875d258a0e055b1f2d23
inline bool test_c74847a08a08875d258a0e055b1f2d23(const int* i) {
    return i != nullptr;
}
#endif
