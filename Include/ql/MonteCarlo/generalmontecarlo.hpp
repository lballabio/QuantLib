
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
/*! \file generalmontecarlo.hpp
    \brief A class useful in many different Monte-Carlo-type simulations

  $Source$
  $Name$
  $Log$
  Revision 1.1  2001/04/09 14:05:48  nando
  all the *.hpp moved below the Include/ql level

  Revision 1.2  2001/04/06 18:46:20  nando
  changed Authors, Contributors, Licence and copyright header

  Revision 1.1  2001/04/04 11:07:22  nando
  Headers policy part 1:
  Headers should have a .hpp (lowercase) filename extension
  All *.h renamed to *.hpp

  Revision 1.3  2001/01/30 15:48:25  marmar
  Template class updated

    Revision 1.2  2001/01/30 09:21:48  marmar
    Documentation updated

    Revision 1.1  2001/01/30 09:03:38  marmar
    GeneralMonteCarlo contains the basic ideas of any Monte Carlo
    simulation: sample from a "sample generator" and accumulate
    in a "sample accumulator".

*/

#ifndef quantlib_general_montecarlo_h
#define quantlib_general_montecarlo_h

#include "ql/qldefines.hpp"

namespace QuantLib {

    namespace MonteCarlo {
    /*!
    Given a sample-accumulator class SA and a sample-generator SG class,
    a GeneralMonteCarlo<SA, SG>  class is constructed. This class
    can be used to sample over the generator and store the results in the
    accumulator. The accumulator itself can be returned upon request.

    The minimal interfaces that SA and SG should implements are

        class SA{
            void add(SAMPLE_TYPE sample, double weight) const;
        };

        class SG{
            SAMPLE_TYPE next() const;
            double weight() const;
        };

    */
        template<class SA, class SG>
        class GeneralMonteCarlo {
        public:
            GeneralMonteCarlo():isInitialized_(false){}
            GeneralMonteCarlo(SA &statisticAccumulator,
                              SG &sampleGenerator);
            SA sampleAccumulator(long iterations = 0) const;
        private:
            mutable bool isInitialized_;
            mutable SA sampleAccumulator_;
            mutable SG sampleGenerator_;
        };

        template<class SA, class SG>
        inline GeneralMonteCarlo<SA, SG>::GeneralMonteCarlo(
                SA &sampleAccumulator, SG &sampleGenerator):
                sampleAccumulator_(sampleAccumulator),
                sampleGenerator_(sampleGenerator),
                isInitialized_(true){}

        template<class SA, class SG>
        inline SA GeneralMonteCarlo<SA, SG>::sampleAccumulator(long iterations) const{
            QL_REQUIRE(isInitialized_ == true,
                       "GeneralMonteCarlo must be initialized");
            for(long j = 1; j <= iterations; j++){
                sampleAccumulator_.add(sampleGenerator_.next(),
                                          sampleGenerator_.weight());
            }
            return sampleAccumulator_;
        }

    }

}
#endif
