
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
    ql/MonteCarlo/%randomarraygenerator.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_random_array_generator_h
#define quantlib_montecarlo_random_array_generator_h

#include "ql/Math/matrix.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates random arrays from a random number generator
        template <class RNG>
        class RandomArrayGenerator {
          public:
            typedef Array SampleType;

            // equal average, equal variance, no covariance
            RandomArrayGenerator(unsigned int dimension,
                                 double average,
                                 double variance,
                                 long seed = 0);
            // different averages, equal variance, no covariance
            RandomArrayGenerator(const Array& average,
                                 double variance,
                                 long seed = 0);
            // equal average, different variances, no covariance
            RandomArrayGenerator(double average,
                                 const Array& variance,
                                 long seed = 0);
            // different averages, different variances, no covariance
            RandomArrayGenerator(const Array& average,
                                 const Array& variance,
                                 long seed = 0);
            // equal average, different variances, covariance
            RandomArrayGenerator(double average,
                                 const Math::Matrix& covariance,
                                 long seed = 0);
            // different averages, different variances, covariance
            RandomArrayGenerator(const Array& average,
                                 const Math::Matrix& covariance,
                                 long seed = 0);
            const Array& next() const;
            double weight() const { return weight_; }
            int size() const { return average_.size(); }
          private:
            mutable Array next_;
            mutable double weight_;
            RNG generator_;
            Array average_, sqrtVariance_;
            Math::Matrix sqrtCovariance_;
        };

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            unsigned int dimension, double average, double variance, 
            long seed) 
        : next_(dimension), generator_(seed), average_(dimension,average) {
            QL_REQUIRE(variance >= 0,
                "RandomArrayGenerator: negative variance");
            sqrtVariance_ = Array(dimension, QL_SQRT(variance));
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            const Array& average, double variance, long seed) 
        : next_(average.size()), generator_(seed), average_(average) {
            QL_REQUIRE(variance >= 0,
                "RandomArrayGenerator: negative variance");
            sqrtVariance_ = Array(average_.size(), QL_SQRT(variance));
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            double average, const Array& variance, long seed) 
        : next_(variance.size()), generator_(seed), 
          average_(variance.size(),average), 
          sqrtVariance_(variance.size()) {
            for (unsigned int i=0; i<variance.size(); i++) {
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
            const Array& average, const Array& variance, long seed) 
        : next_(average.size()), generator_(seed), average_(average),
          sqrtVariance_(variance.size()) {
            for (unsigned int i=0; i<variance.size(); i++) {
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
            double average, const Math::Matrix& covariance, long seed)
        : next_(covariance.rows()), generator_(seed), 
          average_(covariance.rows(),average) {
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Covariance matrix must be square (is "+
                IntegerFormatter::toString(covariance.rows())+ " x "+
                IntegerFormatter::toString(covariance.columns())+ ")");
            QL_REQUIRE(covariance.rows() > 0, 
                "Null covariance matrix given");
            sqrtCovariance_ = Math::matrixSqrt(covariance);
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            const Array& average, const Math::Matrix &covariance, long seed)
        : next_(average.size()), generator_(seed), average_(average) {
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Covariance matrix must be square (is "+
                IntegerFormatter::toString(covariance.rows())+ " x "+
                IntegerFormatter::toString(covariance.columns())+ ")");
            QL_REQUIRE(covariance.rows() > 0, 
                "Null covariance matrix given");
            QL_REQUIRE(average_.size() == covariance.rows(),
                "average-vector size ("+
                IntegerFormatter::toString(average_.size())+ ") "+
                "does not match covariance matrix size("+
                IntegerFormatter::toString(covariance.columns())+ ")");
            sqrtCovariance_ = Math::matrixSqrt(covariance);
        }


        template <class RNG>
        inline const Array& RandomArrayGenerator<RNG>::next() const{
            // starting point for product
            weight_ = 1.0;

            if (sqrtCovariance_.rows() != 0) {  // general case
                for (unsigned int j=0; j<next_.size(); j++) {
                    next_[j] = generator_.next();
                    weight_ *= generator_.weight();
                }
                next_ = average_ + sqrtCovariance_ * next_;

            } else {                            // degenerate case
                for (unsigned int j=0; j<next_.size(); j++){
                    next_[j] = average_[j] +
                               generator_.next() * sqrtVariance_[j];
                    weight_ *= generator_.weight();
                }
            }

            return next_;
        }

    }

}

#endif
