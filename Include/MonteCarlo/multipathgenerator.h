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
 * QuantLib license is also available at 
 *          http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file multipathgenerator.h
    \brief Generates a multi path from a random-point generator

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/25 16:11:17  marmar
    MultiPath, the first step for a multi-dimensional Monte Carlo


*/

#ifndef quantlib_montecarlo_multi_path_generator_h
#define quantlib_montecarlo_multi_path_generator_h

#include "qldefines.h"
#include "multipath.h"
#include "matrix.h"
#include <iostream>

namespace QuantLib {

    namespace MonteCarlo {
    /*!
    MultiPathGenerator<RP> returns a multi path starting from a random point,
    where RP is a sample generator which returns a random point
    */

        template <class RP>
        class MultiPathGenerator {
        public:
        // typedef MultiPath SampleType;
        // this typedef would make MultiPathGenerator into a sample generator
            MultiPathGenerator() : 
                independentVariables_(0), timeDimesion_(0), weight_(0) {}
            MultiPathGenerator(int timeDimension,            
                    Array average, Math::Matrix covariance, long seed=0);
            const MultiPath & next() const;
            const double weight() const{return weight_;}
        private:
            int independentVariables_, timeDimesion_;
            RP rndPoint;
            mutable double weight_;
            mutable Array average_;
            mutable Math::Matrix generatingMatrix_;
            mutable MultiPath multiPath_;
        };

        template <class RP>
        inline MultiPathGenerator<RP >::MultiPathGenerator(
            int timeDimension, Array average, 
            Math::Matrix covariance, long seed): 
            independentVariables_(covariance.rows()),
            timeDimesion_(timeDimension),
            multiPath_(covariance.rows(), timeDimension), 
            average_(average),             
            generatingMatrix_(covariance.rows(),covariance.rows()),
            rndPoint(seed){
                            
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Covariance matrix must be square ("+
                DoubleFormatter::toString(covariance.rows())+ ", "+
                DoubleFormatter::toString(covariance.columns())+ ")");

            QL_REQUIRE(independentVariables_ > 0,
                "Number of indepente variables("+
                DoubleFormatter::toString(independentVariables_)+
                ") too small");
            QL_REQUIRE(timeDimesion_ > 0,
                "Time dimension("+
                DoubleFormatter::toString(timeDimesion_)+
                ") too small");

            QL_REQUIRE(average_.size() == independentVariables_,
                "average-vector size ("+
                DoubleFormatter::toString(average_.size())+ ") "+
                "does not match covariance matrix size("+
                DoubleFormatter::toString(covariance.columns())+ ")");
            
            generatingMatrix_ = sqrt(covariance);                  
            
              std::cout << "generatingMatrix" <<std::endl;
              for(int i=0; i<generatingMatrix_.rows();i++){
                  for(int j=0; j<generatingMatrix_.columns();j++)
                      std::cout << generatingMatrix_[i][j] << "  ";
                  std::cout << std::endl;      
              }            
              std::cout << std::endl;
              
        }

        template <class RP>
        inline const MultiPath & MultiPathGenerator<RP >::next() const{

            QL_REQUIRE(timeDimesion_ > 0,
                    "template MultiPathGenerator: dimension must be positive");
            
            Array disp(independentVariables_);     
            Array resu(independentVariables_);     
                    
            weight_ = 1.0;
            for(int i = 0; i < timeDimesion_;i++){
                for(int j = 0; j < independentVariables_; j++){
                    disp[j] = rndPoint.next();
                    weight_ *= rndPoint.weight();
                }                 
                
                resu = average_ + generatingMatrix_ * disp;
                                    
                for(int j = 0; j < independentVariables_; j++){
                    multiPath_[j][i] = resu[j]; 
                }
            }
            return multiPath_;
        }

    }

}

#endif
