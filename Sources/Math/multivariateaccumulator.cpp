
/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file multivariateaccumulator.cpp

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/24 10:51:16  marmar
    Sample accumulator for multivariate analisys


*/

#include "multivariateaccumulator.h"
#include "outerproduct.h"

namespace QuantLib {

    namespace Math {

        MultivariateAccumulator::MultivariateAccumulator()
                : size_(0){
            reset();            
        }

        MultivariateAccumulator::MultivariateAccumulator(int size):
                size_(size){
            QL_REQUIRE(size_ > 0,
                    "MultivariateAccumulator: size must be positive");
            reset();            
        }

        void MultivariateAccumulator::reset() {
            sampleNumber_ = 0;
            sampleWeight_ = 0.0;
            sum_ = Array(size_,0.0);
            quadraticSum_ = Matrix(size_, size_, 0.0);
        }
        

        void MultivariateAccumulator::add(Array value, double weight) {
        /*! \pre weights must be positive or null */
            QL_REQUIRE(weight >= 0.0,
                "MultivariateAccumulator::add : negative weight (" +
                DoubleFormatter::toString(weight) + ") not allowed");
            QL_REQUIRE(value.size() == size_,
                "MultivariateAccumulator::add : wrong size for input array");
                
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
          return inv*(quadraticSum_ - inv*outerProduct(sum_,sum_) );
        }

        std::vector<double> MultivariateAccumulator::meanVector() const {
          
            Array ma(meanArray());
            std::vector<double> mv(ma.size());            
            std::copy(ma.begin(), ma.end(), mv.begin());            
            return mv;
        }
        
        void MultivariateAccumulator::add(std::vector<double> vec, double wei){
                  
            Array arr(vec.size());
            std::copy(vec.begin(), vec.end(), arr.begin());
            add(arr,wei);
        }

        template <class DataIterator>
        void MultivariateAccumulator::addSequence(DataIterator begin, 
            DataIterator end) {
            for (;begin!=end;++begin)                
                add(*begin);
        }
        
        template <class DataIterator, class WeightIterator>
        void MultivariateAccumulator::addSequence(DataIterator begin, 
                                DataIterator end, WeightIterator wbegin) {
                                    
            for(;begin!=end;++begin,++wbegin)
                add(*begin, *wbegin);
        }        
                
    }

}
