
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file multivariateaccumulator.cpp
    \brief A simple accumulator for vector-type samples
*/

// $Id$

#include <ql/Math/multivariateaccumulator.hpp>

namespace QuantLib {

    namespace Math {

        MultivariateAccumulator::MultivariateAccumulator()
        : size_(0) {
            reset();
        }

        MultivariateAccumulator::MultivariateAccumulator(Size size)
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
            } else {
                QL_REQUIRE(value.size() == size_,
                        "MultivariateAccumulator::add : "
                                    "wrong size for input array");
            }

            QL_REQUIRE(weight >= 0.0,
                "MultivariateAccumulator::add : negative weight (" +
                DoubleFormatter::toString(weight) + ") not allowed");

            Size oldSamples = sampleNumber_;
            sampleNumber_++;
            QL_ENSURE(sampleNumber_ > oldSamples,
                "MultivariateAccumulator::add : "
                "maximum number of samples reached");

            sampleWeight_ += weight;

            Array weighedValue = weight*value;

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


        Matrix MultivariateAccumulator::correlation() const {
          Matrix correlation = covariance();
          Array variances = correlation.diagonal();
          Size dimension = variances.size();
          for (Size i=0; i < dimension; ++i){
              for (Size j=0 ; j < dimension; ++j){
                  if( i == j){
                      if(variances[i] == 0.0){
                      
                          correlation[i][j] = 1.0;
                      }
                      else{
                          
                          correlation[i][j] *=1.0/QL_SQRT(variances[i]*variances[j]);
                      }

                  }
                  else{

                      if(variances[i] == 0.0 && variances[j] == 0){
                          
                          correlation[i][j] = 1.0 ;
                      }
                      else if(variances[i] == 0.0 || variances[j] == 0.0){
                          
                          correlation[i][j] = 0.0 ;   
                      }  
                      else{                            
                           correlation[i][j] *= 1.0/QL_SQRT(variances[i]*variances[j]);   
                      }
                  }
              }
          }
          return correlation;
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
          add(arr, wei);
        }

    }

}
