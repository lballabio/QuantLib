
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/
/*! \file randomarraygenerator.hpp
    \brief A random array generator

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/02/16 15:37:54  lballabio
    renamed sqrt to matrixSqrt

    Revision 1.3  2001/02/06 17:07:17  marmar
    New constructor added

    Revision 1.2  2001/02/05 13:52:47  lballabio
    Makefiles fixed

    Revision 1.1  2001/02/02 10:20:18  marmar
    The class RandomArrayGenerator is a generalization
    of the template class PathGenerator

*/

#ifndef quantlib_montecarlo_random_array_generator_h
#define quantlib_montecarlo_random_array_generator_h

#include "ql/qldefines.hpp"
#include "ql/qlerrors.hpp"
#include "ql/array.hpp"
#include "ql/Math/matrix.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

    /*!
    RandomArrayGenerator<RP> is a template class which returns a random array
    from a random number generator class RP.
    */
        template <class RP>
        class RandomArrayGenerator {
        public:
        // typedef Array SampleType;
        // this typedef would make RandomArrayGenerator into a sample generator
            RandomArrayGenerator();
            RandomArrayGenerator(int dimension, double average = 0.0,
                          double stddev = 1.0, long seed=0);
            RandomArrayGenerator(const Math::Matrix &covariance, long seed=0);
            RandomArrayGenerator(const Array &average,
                         const Math::Matrix &covariance, long seed=0);
            Array next() const;
            double weight() const{return weight_;}
        private:
            int size_;
            RP rndPoint_;
            mutable double weight_;
            double average_, sqrtVariance_;
            mutable Array averageArray_;
            mutable Math::Matrix sqrtCovariance_;
        };

        template <class RP>
        inline RandomArrayGenerator<RP >::RandomArrayGenerator():
                size_(0), weight_(0) {}

        template <class RP>
        inline RandomArrayGenerator<RP >::RandomArrayGenerator(int dimension,
                double average, double variance, long seed):
                size_(dimension), average_(average), rndPoint_(seed),
                averageArray_(0),sqrtCovariance_(0,0){
            QL_REQUIRE(variance >= 0,
                    "RandomArrayGenerator: variance is negative!");
            sqrtVariance_ = QL_SQRT(variance);
        }

        template <class RP>
        inline RandomArrayGenerator<RP >::RandomArrayGenerator(
            const Math::Matrix &covariance, long seed):
            size_(covariance.rows()), averageArray_(covariance.rows(),0),
            sqrtCovariance_(covariance.rows(),covariance.rows()),
            rndPoint_(seed){

            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Covariance matrix must be square ("+
                DoubleFormatter::toString(covariance.rows())+ ", "+
                DoubleFormatter::toString(covariance.columns())+ ")");

            QL_REQUIRE(size_ > 0,
                "Number of indepente variables("+
                DoubleFormatter::toString(size_)+
                ") too small");

            sqrtCovariance_ = Math::matrixSqrt(covariance);
        }

        template <class RP>
        inline RandomArrayGenerator<RP >::RandomArrayGenerator(
            const Array &average, const Math::Matrix &covariance, long seed):
                size_(covariance.rows()), averageArray_(average),
            sqrtCovariance_(covariance.rows(),covariance.rows()),
            rndPoint_(seed){

            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Covariance matrix must be square ("+
                DoubleFormatter::toString(covariance.rows())+ ", "+
                DoubleFormatter::toString(covariance.columns())+ ")");

            QL_REQUIRE(size_ > 0,
                "Number of indepente variables("+
                DoubleFormatter::toString(size_)+
                ") too small");

            QL_REQUIRE(averageArray_.size() == size_,
                "average-vector size ("+
                DoubleFormatter::toString(averageArray_.size())+ ") "+
                "does not match covariance matrix size("+
                DoubleFormatter::toString(covariance.columns())+ ")");

            sqrtCovariance_ = Math::matrixSqrt(covariance);
        }

        template <class RP>
        inline Array RandomArrayGenerator<RP >::next() const{

            Array nextArray(size_);

            weight_ = 1.0;
            for(int j = 0; j < size_; j++){
                nextArray[j] = rndPoint_.next();
                weight_ *= rndPoint_.weight();
            }

            if(averageArray_.size() == 0)
                nextArray = average_ + sqrtVariance_ * nextArray;
            else
                nextArray = averageArray_ + sqrtCovariance_ * nextArray;

            return nextArray;
        }

    }

}

#endif
