
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file randomarraygenerator.hpp
    \brief Generates random arrays from a random number generator
*/

#ifndef quantlib_montecarlo_random_array_generator_h
#define quantlib_montecarlo_random_array_generator_h

#include <ql/MonteCarlo/sample.hpp>
#include <ql/Math/pseudosqrt.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    //! Generates random arrays using a random number generator
    /*! \deprecated use RandomSequenceGenerator instead. */
    template <class RNG>
    class RandomArrayGenerator {
      public:
        typedef Sample<Array> sample_type;
        // equal average, different variances, no covariance
        RandomArrayGenerator(const Array& variance,
                             long seed = 0);
        // different averages, different variances, covariance
        RandomArrayGenerator(const Matrix& covariance,
                             long seed = 0);
        const sample_type& next() const;
        int size() const { return next_.size(); }
      private:
        mutable sample_type next_;
        RNG generator_;
        Array sqrtVariance_;
        Matrix sqrtCovariance_;
    };


    template <class RNG>
    RandomArrayGenerator<RNG>::RandomArrayGenerator(
                                             const Array& variance, long seed)
    : next_(Array(variance.size()),1.0), 
      generator_(typename RNG::urng_type(seed)),
      sqrtVariance_(variance.size()) {
        for (Size i=0; i<variance.size(); i++) {
            QL_REQUIRE(variance[i] >= 0,
                       "RandomArrayGenerator: negative variance"
                       + DoubleFormatter::toString(variance[i])
                       + "in position "
                       + IntegerFormatter::toString(i));
            sqrtVariance_[i] = QL_SQRT(variance[i]);
        }
    }

    template <class RNG>
    RandomArrayGenerator<RNG>::RandomArrayGenerator(
                                          const Matrix& covariance, long seed)
    : next_(Array(covariance.rows()),1.0), 
      generator_(typename RNG::urng_type(seed)) {
        QL_REQUIRE(covariance.rows() == covariance.columns(),
                   "Covariance matrix must be square (is "+
                   IntegerFormatter::toString(covariance.rows())+ " x "+
                   IntegerFormatter::toString(covariance.columns())+ ")");
        QL_REQUIRE(covariance.rows() > 0,
                   "Null covariance matrix given");
        sqrtCovariance_ = pseudoSqrt(covariance, SalvagingAlgorithm::None);
    }


    template <class RNG>
    const typename RandomArrayGenerator<RNG>::sample_type&
    RandomArrayGenerator<RNG>::next() const {
        // starting point for product
        next_.weight = 1.0;

        for (Size j=0; j<next_.value.size(); j++) {
            typename RNG::sample_type sample = generator_.next();
            next_.value[j] = sample.value;
            next_.weight *= sample.weight;
        }

        if (sqrtCovariance_.rows() != 0) {  // general case
            next_.value = sqrtCovariance_ * next_.value;
        } else {                            // degenerate case
            for (Size j=0; j<next_.value.size(); j++)
                next_.value[j] *= sqrtVariance_[j];
        }
        return next_;
    }

}


#endif
