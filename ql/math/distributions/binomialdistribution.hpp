/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file binomialdistribution.hpp
    \brief Binomial distribution
*/

#ifndef quantlib_binomial_distribution_h
#define quantlib_binomial_distribution_h

#include <ql/math/factorial.hpp>
#include <ql/math/beta.hpp>

namespace QuantLib {

    inline Real binomialCoefficientLn(BigNatural n, BigNatural k) {

        QL_REQUIRE(n>=k, "n<k not allowed");

        return Factorial::ln(n)-Factorial::ln(k)-Factorial::ln(n-k);

    }

    inline Real binomialCoefficient(BigNatural n, BigNatural k) {

        return std::floor(0.5+std::exp(binomialCoefficientLn(n, k)));

    }

    //! Binomial probability distribution function
    /*! formula here ...
        Given an integer k it returns its probability in a Binomial
        distribution with parameters p and n.
    */
    class BinomialDistribution {
      public:
        BinomialDistribution(Real p, BigNatural n);
        // function
        Real operator()(BigNatural k) const;
      private:
        BigNatural n_;
        Real logP_, logOneMinusP_;
    };

    //! Cumulative binomial distribution function
    /*! Given an integer k it provides the cumulative probability
        of observing kk<=k:
        formula here ...

    */
    class CumulativeBinomialDistribution {
      public:
        CumulativeBinomialDistribution(Real p, BigNatural n);
        // function
        Real operator()(BigNatural k) const {
            if (k >= n_)
                return 1.0;
            else
                return 1.0 - incompleteBetaFunction(k+1, n_-k, p_);
        }
      private:
        BigNatural n_;
        Real p_;
    };


    inline BinomialDistribution::BinomialDistribution(Real p,
                                                      BigNatural n)
    : n_(n) {

        if (p==0.0) {
            logP_ = -QL_MAX_REAL;
            logOneMinusP_ = 0.0;
        } else if (p==1.0) {
            logP_ = 0.0;
            logOneMinusP_ = -QL_MAX_REAL;
        } else {
            QL_REQUIRE(p>0, "negative p not allowed");
            QL_REQUIRE(p<1.0, "p>1.0 not allowed");

            logP_ = std::log(p);
            logOneMinusP_ = std::log(1.0-p);
        }
    }


    inline
    CumulativeBinomialDistribution::CumulativeBinomialDistribution(
                                                       Real p, BigNatural n)
    : n_(n), p_(p) {

        QL_REQUIRE(p>=0, "negative p not allowed");
        QL_REQUIRE(p<=1.0, "p>1.0 not allowed");

    }

    inline Real BinomialDistribution::operator()(BigNatural k) const {

        if (k > n_) return 0.0;

        // p==1.0
        if (logP_==0.0)
            return (k==n_ ? 1.0 : 0.0);
        // p==0.0
        else if (logOneMinusP_==0.0)
            return (k==0 ? 1.0 : 0.0);
        else
            return std::exp(binomialCoefficientLn(n_, k) +
                            k * logP_ + (n_-k) * logOneMinusP_);
    }



    /*! Given an odd integer n and a real number z it returns p such that:
        1 - CumulativeBinomialDistribution((n-1)/2, n, p) =
                               CumulativeNormalDistribution(z)

        \pre n must be odd
    */
    inline Real PeizerPrattMethod2Inversion(Real z, BigNatural n) {

        QL_REQUIRE(n%2==1,
                   "n must be an odd number: " << n << " not allowed");

        Real result = (z/(n+1.0/3.0+0.1/(n+1.0)));
        result *= result;
        result = std::exp(-result*(n+1.0/6.0));
        result = 0.5 + (z>0 ? 1 : -1) * std::sqrt((0.25 * (1.0-result)));
        return result;
    }

}


#endif
