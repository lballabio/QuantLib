
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
    \brief Generates random arrays from a random number generator

    $Id$
*/

// $Source$
// $Log$
// Revision 1.11  2001/07/06 09:15:36  nando
// style enforced
//
// Revision 1.10  2001/07/06 08:08:43  aleppo
// Bug fixed
//
// Revision 1.9  2001/07/05 13:51:04  nando
// Maxim "Ronin" contribution on efficiency and style
//
// Revision 1.8  2001/07/04 12:00:37  uid40428
// Array of random numbers built with an array of dates

#ifndef quantlib_montecarlo_random_array_generator_h
#define quantlib_montecarlo_random_array_generator_h

#include "ql/Math/matrix.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates random arrays from a random number generator
        template <class RP>
        class RandomArrayGenerator {
          public:
            // typedef Array SampleType;
            // this typedef would make RandomArrayGenerator into a sample
            // generator
            RandomArrayGenerator();

            RandomArrayGenerator(int dimension,
                                 double average = 0.0,
                                 double variance = 1.0,
                                 long seed=0);
            RandomArrayGenerator(const std::vector<Time> & dates,
                                 double average = 0.0,
                                 double variance = 1.0, 
                                 long seed=0);
            RandomArrayGenerator(const Math::Matrix &covariance, 
                                 long seed=0);
            RandomArrayGenerator(const Array &average,
                                 const Math::Matrix &covariance,
                                 long seed=0);
            Array next() const;
            double weight() const{return weight_;}
            int size() const{return size_;}
          private:
            int size_;
            std::vector<Time> timeDelays_;
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
                size_(dimension),
                timeDelays_(dimension, 1.0),
                average_(average), rndPoint_(seed),
                averageArray_(0),sqrtCovariance_(0,0){
            QL_REQUIRE(variance >= 0,
                    "RandomArrayGenerator: variance is negative!");
            sqrtVariance_ = QL_SQRT(variance);
        }

        template <class RP>
        inline RandomArrayGenerator<RP>::RandomArrayGenerator(
            const std::vector<Time> & dates,
            double average,
            double variance,
            long seed)
        :size_(dates.size()), timeDelays_(dates.size()),average_(average),
         rndPoint_(seed), averageArray_(0),sqrtCovariance_(0,0) {

            QL_REQUIRE(variance >= 0,
                    "RandomArrayGenerator: variance is negative!");
            sqrtVariance_ = QL_SQRT(variance);

            if( size_ > 0){
                QL_REQUIRE(dates[0] >= 0,
                     "MultiPathGenerator: first date(" +
                    DoubleFormatter::toString(dates[0])+
                    ") must be positive");
                timeDelays_[0] = dates[0];
            }
            
            if(size_ > 1){
                for(int i = 1; i < size_; i++){
                    QL_REQUIRE(dates[i] >= dates[i-1],
                        "MultiPathGenerator: date(" +
                        IntegerFormatter::toString(i-1)+")="+
                        DoubleFormatter::toString(dates[i-1])+
                        " is later than date("+
                        IntegerFormatter::toString(i)+")="+
                        DoubleFormatter::toString(dates[i]));
                    timeDelays_[i] = dates[i] - dates[i-1];
                }
            }
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

            if(averageArray_.size() == 0){
                for(int j = 0; j < size_; j++){
                    nextArray[j] = average_ * timeDelays_[j]
                                + rndPoint_.next() * sqrtVariance_ *
                                                     QL_SQRT(timeDelays_[j]);
                    weight_ *= rndPoint_.weight();
                }
            }
            else{
                for(int j = 0; j < size_; j++){
                    nextArray[j] = rndPoint_.next();
                    weight_ *= rndPoint_.weight();
                }
                nextArray = averageArray_ + sqrtCovariance_ * nextArray;

            }

            return nextArray;
        }

    }

}

#endif
