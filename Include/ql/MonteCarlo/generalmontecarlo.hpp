
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
    \brief Generic model for MonteCarlo simulations

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/07/13 14:29:08  sigmud
// removed a few gcc compile warnings
//
// Revision 1.8  2001/07/09 16:29:27  lballabio
// Some documentation and market element
//
// Revision 1.7  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.6  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_general_montecarlo_h
#define quantlib_general_montecarlo_h

#include "ql/qldefines.hpp"

namespace QuantLib {

    //! Framework for %MonteCarlo models
    /*! \htmlonly
        <table width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr><td width="50%">&nbsp</td>
        <td>Anyone attempting to generate random numbers by 
        deterministic means is, of course, living in a state of sin.</td></tr>
        <tr><td width="50%">&nbsp</td>
        <td align="right">John von Neumann</td></tr>
        </table>
        \endhtmlonly
        
        \latexonly
        \begin{quotation}
            \small Anyone attempting to generate random numbers by 
            deterministic means is, of course, living in a state of sin. 
            \hfill --- John Von Neumann
        \end{quotation}
        \endlatexonly
        
        This namespace contains basic building blocks for the construction 
        of MonteCarlo models, namely,
        
        - a generic model, GeneralMonteCarlo,
        - a number of uniform and Gaussian random number and array generators,
        - a Path class which represents a random path,
        
        as well as more specialized classes.
    */
    namespace MonteCarlo {

        //! generic %MonteCarlo model
        /*! Given a sample-accumulator class SA and a sample-generator SG class,
            a GeneralMonteCarlo<SA, SG>  class is constructed. This class
            can be used to sample over the generator and store the results in 
            the accumulator. The accumulator itself can be returned upon 
            request.
    
            The minimal interfaces that SA and SG should implements are
    
            \code
            class SA{
                void add(SAMPLE_TYPE sample, double weight) const;
            };
    
            class SG{
                SAMPLE_TYPE next() const;
                double weight() const;
            };
            \endcode
        */
        template<class SA, class SG>
        class GeneralMonteCarlo {
          public:
            GeneralMonteCarlo() : isInitialized_(false) {}
            GeneralMonteCarlo(SA &statisticAccumulator,
                              SG &sampleGenerator);
            SA sampleAccumulator(long iterations = 0) const;
          private:
            mutable SA sampleAccumulator_;
            mutable SG sampleGenerator_;
            mutable bool isInitialized_;
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
