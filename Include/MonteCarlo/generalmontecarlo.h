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
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file generalmontecarlo.h
    \brief A class useful in many different Monte-Carlo-type simulations

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/30 09:03:38  marmar
    GeneralMonteCarlo contains the basic ideas of any Monte Carlo
    simulation: sample from a "sample generator" and accumulate
    in a "sample accumulator".

*/

#ifndef quantlib_general_montecarlo_h
#define quantlib_general_montecarlo_h

#include "qldefines.h"

namespace QuantLib {

    namespace MonteCarlo {
    /*!
    Given a sample accumulator class SA, together with an instance 
    "statisticAccumulator", and a sample generator SG, again with an 
    instance "sampleGenerator", a class GeneralMonteCarlo<SA, SG> 
    
     returns repeatedky loops over the generator
    at each next a value
    for the option price.

    Minimal interfaces for SA and SG:

    class SA{
        SAMPLE_TYPE next() const;
        double weight() const;
    };

    class SG{
        double add(SAMPLE_TYPE sample, double weight) const;    
    };
    */

        template<class SA, class SG>
        class GeneralMonteCarlo {
        public:
            GeneralMonteCarlo(){}
            GeneralMonteCarlo(SA &statisticAccumulator, 
                              SG &sampleGenerator);
            void sample(long iterations) const; 
            SA statisticAccumulator() const;
        private:
            mutable SA statisticAccumulator_;
            mutable SG sampleGenerator_;
        };

        template<class SA, class SG>
        inline GeneralMonteCarlo<SA, SG>::GeneralMonteCarlo(
                SA &statisticAccumulator, SG &sampleGenerator): 
                statisticAccumulator_(statisticAccumulator),
                sampleGenerator_(sampleGenerator){}

        template<class SA, class SG>
        inline void GeneralMonteCarlo<SA, SG>::sample(long iterations) const{
            for(long j = 1; j <= iterations; j++){
                statisticAccumulator_.add(sampleGenerator_.next(),
                                          sampleGenerator_.weight());
            }                                            
        }

        template<class SA, class SG>
        inline SA GeneralMonteCarlo<SA, SG>::statisticAccumulator() const{
            return statisticAccumulator_;
        }

    }

}
#endif
