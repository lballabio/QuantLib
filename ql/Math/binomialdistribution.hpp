
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

/*! \file binomialdistribution.hpp
    \brief Binomial distribution
*/

#ifndef quantlib_binomial_distribution_h
#define quantlib_binomial_distribution_h

#include <ql/Math/factorial.hpp>
#include <ql/Math/beta.hpp>

namespace QuantLib {

    inline double binomialCoefficientLn(unsigned long n, unsigned long k) {

        QL_REQUIRE(n>=0,
            "binomialCoefficient: "
            "negative n not allowed");

        QL_REQUIRE(k>=0,
            "binomialCoefficient: "
            "negative k not allowed");

        QL_REQUIRE(n>=k,
            "binomialCoefficient: "
            "n<k not allowed");

        return Factorial::ln(n)-Factorial::ln(k)-Factorial::ln(n-k);

    }

    inline double binomialCoefficient(unsigned long n, unsigned long k) {

        return QL_FLOOR(0.5+QL_EXP(binomialCoefficientLn(n, k)));

    }

    //! Binomial probability distribution function
    /*! formula here ...
        Given an integer k it returns its probability in a Binomial
        distribution with parameters p and n.
    */
    class BinomialDistribution : public std::unary_function<double,double> {
      public:
        BinomialDistribution(double p, unsigned long n);
        // function
        double operator()(unsigned long k) const;
      private:
        unsigned long n_;
        double logP_, logOneMinusP_;
    };

    //! Cumulative binomial distribution function
    /*! Given an integer k it provides the cumulative probability
        of observing kk<=k:
        formula here ...

    */
    class CumulativeBinomialDistribution
    : public std::unary_function<double,double> {
      public:
        CumulativeBinomialDistribution(double p, unsigned long n);
        // function
        double operator()(unsigned long k) const {
            if (k >= n_) return 1.0;
            QL_REQUIRE(k>=0,
                "CumulativeBinomialDistribution::operator() : "
                "negative k not allowed");
            return 1.0 - incompleteBetaFunction(k+1, n_-k, p_);
        }
      private:
        unsigned long n_;
        double p_;
    };


    inline BinomialDistribution::BinomialDistribution(double p, 
                                                      unsigned long n)
    : n_(n) {

        QL_REQUIRE(n>=0,
            "BinomialDistribution::BinomialDistribution : "
            "negative n not allowed");

        if (p==0.0) {
            logOneMinusP_ = 0.0;
        } else if (p==1.0) {
            logP_ = 0.0;
        } else {
            QL_REQUIRE(p>0,
                "BinomialDistribution::BinomialDistribution : "
                "negative p not allowed");

            QL_REQUIRE(p<1.0,
                "BinomialDistribution::BinomialDistribution : "
                "p>1.0 not allowed");

            logP_ = QL_LOG(p);
            logOneMinusP_ = QL_LOG(1.0-p);
        }
    }


    inline 
    CumulativeBinomialDistribution::CumulativeBinomialDistribution(double p,
        unsigned long n)
    : n_(n), p_(p) {

        QL_REQUIRE(n>=0,
            "CumulativeBinomialDistribution::CumulativeBinomialDistribution : "
            "negative n not allowed");

        QL_REQUIRE(p>=0,
            "CumulativeBinomialDistribution::CumulativeBinomialDistribution : "
            "negative p not allowed");

        QL_REQUIRE(p<=1.0,
            "CumulativeBinomialDistribution::CumulativeBinomialDistribution : "
            "p>1.0 not allowed");

    }

    inline double BinomialDistribution::operator()(unsigned long k) const {

        if (k > n_) return 0.0;
        QL_REQUIRE(k>=0,
            "BinomialDistribution::operator() : "
            "negative k not allowed");

        // p==1.0
        if (logP_==0.0)
            return (k==n_ ? 1.0 : 0.0);
        // p==0.0
        else if (logOneMinusP_==0.0)
            return (k==0 ? 1.0 : 0.0);
        else
            return QL_EXP(binomialCoefficientLn(n_, k) +
            k * logP_ + (n_-k) * logOneMinusP_);
    }



    /*! Given an odd integer n and a real number z it returns p such that:
        1 - CumulativeBinomialDistribution((n-1)/2, n, p) =
                               CumulativeNormalDistribution(z)

        \pre n must be odd
    */
    inline double PeizerPrattMethod2Inversion(double z, unsigned long n) {

        QL_REQUIRE(n%2==1,
            "PeizerPrattMethod2Inversion : "
            "n must be an odd number" +
            IntegerFormatter::toString(n) + " not allowed");

        double result = (z/(n+1.0/3.0+0.1/(n+1.0)));
        result *= result;
        result = QL_EXP(-result*(n+1.0/6.0));
        result = 0.5 + (z>0 ? 1 : -1) * QL_SQRT((0.25 * (1.0-result)));
        return result;
    }

}


#endif
