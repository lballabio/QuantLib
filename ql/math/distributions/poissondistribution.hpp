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
 <https://www.quantlib.org/license.shtml>.

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
#include <limits>

namespace QuantLib {

    //! Poisson distribution function
    /*! Given an integer \f$ k \f$, it returns its probability
        in a Poisson distribution.

        \test the correctness of the returned value is tested by
              checking it against known good results.
    */
    class PoissonDistribution {
      public:
        // TODO: Review whether this constructor should remain implicit.
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
        // TODO: Review whether this constructor should remain implicit.
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
        // TODO: Review whether this constructor should remain implicit.
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
        if (x == 0.0)
            return 0.0;

        if (x <= std::exp(-lambda_))
            return 0.0;

        // Indices around lambda must fit both the counter and Factorial::ln.
        QL_REQUIRE(lambda_ < Real((std::numeric_limits<BigNatural>::max)()) &&
                       lambda_ < Real((std::numeric_limits<Natural>::max)()),
                   "lambda (" << lambda_ << ") exceeds the supported index range");

        if (x > 0.5) {
            // Sum the survival probability from the right to avoid cancellation near one.
            Real tail = 1.0-x;
            BigNatural index = static_cast<BigNatural>(lambda_);
            // Above the mode, a geometric series bounds the remaining probability.
            while (calcSummand(index) / (1.0-lambda_/(index+1.0)) > tail*QL_EPSILON)
                ++index;

            Real sum = 0.0;
            while (index > 0) {
                sum += calcSummand(index);
                if (sum > tail)
                    return Real(index);
                --index;
            }
            return 0.0;
        }

        Real sum = 0.0;
        BigNatural index = 0;
        while (x > sum) {
            sum += calcSummand(index);
            index++;
        }

        return Real(index-1);
    }

    inline Real InverseCumulativePoisson::calcSummand(BigNatural index) const {
        QL_REQUIRE(index < (std::numeric_limits<BigNatural>::max)() &&
                       index < (std::numeric_limits<Natural>::max)(),
                   "Poisson index exceeds the supported range");
        return std::exp(index*std::log(lambda_) - Factorial::ln(Natural(index)) - lambda_);
    }

}


#endif
