
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
            typedef Array sample_type;
            // equal average, equal variance, no covariance
            RandomArrayGenerator(unsigned int dimension,
                                 double average,
                                 double variance,
                                 bool antithetic,
                                 long seed = 0);
            // different averages, equal variance, no covariance
            RandomArrayGenerator(const Array& average,
                                 double variance,
                                 bool antithetic,
                                 long seed = 0);
            // equal average, different variances, no covariance
            RandomArrayGenerator(double average,
                                 const Array& variance,
                                 bool antithetic,
                                 long seed = 0);
            // different averages, different variances, no covariance
            RandomArrayGenerator(const Array& average,
                                 const Array& variance,
                                 bool antithetic,
                                 long seed = 0);
            // equal average, different variances, covariance
            RandomArrayGenerator(double average,
                                 const Math::Matrix& covariance,
                                 bool antithetic,
                                 long seed = 0);
            // different averages, different variances, covariance
            RandomArrayGenerator(const Array& average,
                                 const Math::Matrix& covariance,
                                 bool antithetic,
                                 long seed = 0);
            const Array& next() const;
            double weight() const { return weight_; }
            int size() const { return average_.size(); }
          private:
            mutable Array next_, randomComponent_;
            mutable double weight_;
            RNG generator_;
            Array average_, sqrtVariance_;
            Math::Matrix sqrtCovariance_;
            bool antithetic_;
            mutable bool nextAntithetic_;
        };

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            unsigned int dimension, double average, double variance, 
            bool antithetic, long seed) 
        : next_(dimension), randomComponent_(dimension),
          generator_(seed), average_(dimension,average),
          antithetic_(antithetic), nextAntithetic_(false) {
            QL_REQUIRE(variance >= 0,
                "RandomArrayGenerator: negative variance");
            sqrtVariance_ = Array(dimension, QL_SQRT(variance));
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            const Array& average, double variance, bool antithetic, long seed) 
        : next_(average.size()), randomComponent_(average.size()),
          generator_(seed), average_(average),
          antithetic_(antithetic), nextAntithetic_(false) {
            QL_REQUIRE(variance >= 0,
                "RandomArrayGenerator: negative variance");
            sqrtVariance_ = Array(average_.size(), QL_SQRT(variance));
        }

        template <class RNG>
        inline RandomArrayGenerator<RNG>::RandomArrayGenerator(
            double average, const Array& variance, bool antithetic, long seed) 
        : next_(variance.size()), randomComponent_(variance.size()),
          generator_(seed), 
          average_(variance.size(),average), 
          sqrtVariance_(variance.size()),
          antithetic_(antithetic), nextAntithetic_(false) {
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
            const Array& average, const Array& variance, bool antithetic, long seed) 
        : next_(average.size()), randomComponent_(average.size()),
          generator_(seed), average_(average),
          sqrtVariance_(variance.size()),
          antithetic_(antithetic), nextAntithetic_(false) {
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
            double average, const Math::Matrix& covariance, bool antithetic, long seed)
        : next_(covariance.rows()), randomComponent_(covariance.rows()),
          generator_(seed), 
          average_(covariance.rows(),average),
          antithetic_(antithetic), nextAntithetic_(false) {
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
            const Array& average, const Math::Matrix &covariance, bool antithetic, long seed)
        : next_(average.size()), randomComponent_(average.size()),
          generator_(seed), average_(average),
          antithetic_(antithetic), nextAntithetic_(false) {
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

            if (nextAntithetic_) {
                nextAntithetic_=false;
                next_ = average_ - randomComponent_;
            } else {
                // starting point for product
                if (antithetic_) {
                    weight_ = 0.5;
                    nextAntithetic_=true;
                } else {
                    weight_ = 1.0;
                }

                for (unsigned int j=0; j<next_.size(); j++) {
                    randomComponent_[j] = generator_.next();
                    weight_ *= generator_.weight();
                }

                if (sqrtCovariance_.rows() != 0) {  // general case
                    randomComponent_ = sqrtCovariance_ * randomComponent_;
                } else {                            // degenerate case
                    for (unsigned int j=0; j<next_.size(); j++)
                        randomComponent_[j] *= sqrtVariance_[j];
                }
                next_ = average_ + randomComponent_;
            }
            
            return next_;
        }

    }

}

#endif
