
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

/*! \file multivariateaccumulator.cpp
    \brief A simple accumulator for vector-type samples

    $Id$
*/

// $Source$
// $Log$
// Revision 1.13  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.12  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Math/multivariateaccumulator.hpp"

namespace QuantLib {

    namespace Math {

        MultivariateAccumulator::MultivariateAccumulator()
                : size_(0){
            reset();
        }

        MultivariateAccumulator::MultivariateAccumulator(int size)
                : size_(size){
            reset();
        }

        void MultivariateAccumulator::reset() {
            sampleNumber_ = 0;
            sampleWeight_ = 0.0;
            sum_ = Array(size_,0.0);
            quadraticSum_ = Matrix(size_, size_, 0.0);
        }


        void MultivariateAccumulator::add(const Array &value, double weight) {
        /*! \pre weights must be positive or null */

            if(size_ == 0){
                size_ = value.size();
                reset();
            }
            else{
                QL_REQUIRE(value.size() == size_,
                        "MultivariateAccumulator::add : "
                                    "wrong size for input array");
            }

            QL_REQUIRE(weight >= 0.0,
                "MultivariateAccumulator::add : negative weight (" +
                DoubleFormatter::toString(weight) + ") not allowed");

            sampleNumber_ += 1.0;
            sampleWeight_ += weight;
            Array weighedValue(weight*value);

            sum_ += weighedValue;
            quadraticSum_ += outerProduct(weighedValue, value);
        }

        Matrix MultivariateAccumulator::covariance() const {
          QL_REQUIRE(sampleWeight_ > 0.0,
            "Stat::variance() : sampleWeight_=0, unsufficient");
          QL_REQUIRE(sampleNumber_ > 1,
            "Stat::variance() : sample number <=1, unsufficient");

          double inv = 1/sampleWeight_;
          return (sampleNumber_/(sampleNumber_-1.0))*
                inv*(quadraticSum_ - inv*outerProduct(sum_,sum_) );
        }

        std::vector<double> MultivariateAccumulator::meanVector() const {

            Array ma(mean());
            std::vector<double> mv(ma.size());
            std::copy(ma.begin(), ma.end(), mv.begin());
            return mv;
        }

        void MultivariateAccumulator::add(const std::vector<double> &vec,
                                                                double wei){
          Array arr(vec.size());
          std::copy(vec.begin(), vec.end(), arr.begin());
//            for(int i=0; i<vec.size(); i++) arr[i] = vec[i] ;
          add(arr, wei);
        }

    }

}
