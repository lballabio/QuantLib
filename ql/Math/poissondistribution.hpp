
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

/*! \file poissondistribution.hpp
    \brief Poisson distribution
*/

#ifndef quantlib_poisson_distribution_h
#define quantlib_poisson_distribution_h

#include <ql/dataformatters.hpp>
#include <ql/Math/factorial.hpp>
#include <ql/Math/incompletegamma.hpp>

namespace QuantLib {

    //! Normal distribution function
    /*! formula here ...
        Given an integer k it returns its probability in a Poisson distribution.
    */
    class PoissonDistribution : public std::unary_function<double,double> {
      public:
        PoissonDistribution(double mu);
        // function
        double operator()(unsigned long k) const;
      private:
        double mu_, logMu_;
    };


    //! Cumulative Poisson distribution function
    /*! Given x it provides an approximation to the
        integral of the Poisson distribution:
        formula here ...

        For this implementation see
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6
    */
    class CumulativePoissonDistribution
    : public std::unary_function<double,double> {
      public:
        CumulativePoissonDistribution(double mu) : mu_(mu) {}
        // function
        double operator()(unsigned long k) const {
            return 1.0 - incompleteGammaFunction(k+1, mu_); }
      private:
        double mu_;
    };



    // inline definitions

    inline PoissonDistribution::PoissonDistribution(double mu)
    : mu_(mu) {

        QL_REQUIRE(mu_>=0.0,
                   "mu must be non negative (" +
                   DecimalFormatter::toString(mu_) + " not allowed)");

        if (mu_!=0.0) logMu_ = QL_LOG(mu_);
    }

    inline double PoissonDistribution::operator()(unsigned long k) const {
        if (mu_==0.0) {
            if (k==0) return 1.0;
            else      return 0.0;
        }
        double logFactorial = Factorial::ln(k); 
        return QL_EXP(k*QL_LOG(mu_) - logFactorial - mu_);
    }

}


#endif
