
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file randomarraygenerator.hpp
    \brief Generates random arrays from a random number generator

    \fullpath
    ql/RandomNumbers/%randomarraygenerator.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_random_array_generator_h
#define quantlib_montecarlo_random_array_generator_h

#include "ql/Math/matrix.hpp"
#include "ql/MonteCarlo/sample.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace RandomNumbers {

        //! Generates random arrays from a random number generator
        template <class RNG>
        class RandomArrayGenerator {
          public:
            typedef MonteCarlo::Sample<Array> sample_type;
            // equal average, equal variance, no covariance
            RandomArrayGenerator(size_t dimension,
                                 double variance,
                                 long seed = 0);
            // equal average, different variances, no covariance
            RandomArrayGenerator(const Array& variance,
                                 long seed = 0);
            // different averages, different variances, covariance
            RandomArrayGenerator(const Math::Matrix& covariance,
                                 long seed = 0);
            const sample_type& next() const;
            int size() const { return average_.size(); }
          private:
            mutable sample_type next_;
            RNG generator_;
            Array sqrtVariance_;
            Math::Matrix sqrtCovariance_;
        };

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            size_t dimension, double variance,
            long seed)
        : next_(Array(dimension),1.0), generator_(seed) {
            QL_REQUIRE(variance >= 0,
                "RandomArrayGenerator: negative variance");
            sqrtVariance_ = Array(dimension, QL_SQRT(variance));
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            const Array& variance, long seed)
        : next_(Array(variance.size()),1.0), generator_(seed),
          sqrtVariance_(variance.size()) {
            for (size_t i=0; i<variance.size(); i++) {
                QL_REQUIRE(variance[i] >= 0,
                    "RandomArrayGenerator: negative variance"
                    + DoubleFormatter::toString(variance[i])
                    + "in position "
                    + IntegerFormatter::toString(i));
                sqrtVariance_[i] = QL_SQRT(variance[i]);
            }
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            const Math::Matrix& covariance, long seed)
        : next_(Array(covariance.rows()),1.0), generator_(seed) {
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Covariance matrix must be square (is "+
                IntegerFormatter::toString(covariance.rows())+ " x "+
                IntegerFormatter::toString(covariance.columns())+ ")");
            QL_REQUIRE(covariance.rows() > 0,
                "Null covariance matrix given");
            sqrtCovariance_ = Math::matrixSqrt(covariance);
        }


        template <class RNG>
        inline const RandomArrayGenerator<RNG>::sample_type& 
        RandomArrayGenerator<RNG>::next() const{
            // starting point for product
            next_.weight = 1.0;

            for (size_t j=0; j<next_.value.size(); j++) {
                typename RNG::sample_type sample = generator_.next();
                next_.value[j] = sample.value;
                next_.weight *= sample.weight;
            }

            if (sqrtCovariance_.rows() != 0) {  // general case
                next_.value = sqrtCovariance_ * next_.value;
            } else {                            // degenerate case
                for (size_t j=0; j<next_.value.size(); j++)
                    next_.value[j] *= sqrtVariance_[j];
            }
            return next_;
        }

    }

}


#endif
