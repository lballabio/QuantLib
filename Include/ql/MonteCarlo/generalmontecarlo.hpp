
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
/*! \file generalmontecarlo.hpp
    \fullpath Include/ql/MonteCarlo/%generalmontecarlo.hpp
    \brief Generic model for Monte Carlo simulations

*/

// $Id$
// $Log$
// Revision 1.17  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.16  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.15  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.14  2001/07/27 14:45:23  marmar
// Method  void ddSamples(long n) added to GeneralMonteCarlo
//
// Revision 1.13  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.12  2001/07/20 13:06:57  marmar
// Monte Carlo interfaces imporved
//
// Revision 1.11  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.10  2001/07/13 15:25:13  marmar
// MonteCarlo interface changed
//
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

    //! Monte Carlo framework
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
            GeneralMonteCarlo();
            GeneralMonteCarlo(const SA &statisticAccumulator,
                              const SG &sampleGenerator);
            void addSamples(long iterations);
            const SA& sampleAccumulator(void) const;
          private:
            SA sampleAccumulator_;
            SG sampleGenerator_;
            bool isInitialized_;
        };

        template<class SA, class SG>
        inline GeneralMonteCarlo<SA, SG>::GeneralMonteCarlo()
                : isInitialized_(false) {}

        template<class SA, class SG>
        inline GeneralMonteCarlo<SA, SG>::GeneralMonteCarlo(
                const SA &sampleAccumulator, 
                const SG &sampleGenerator):
                sampleAccumulator_(sampleAccumulator),
                sampleGenerator_(sampleGenerator),
                isInitialized_(true){}

        template<class SA, class SG>
        void GeneralMonteCarlo<SA, SG>::addSamples(long iterations) {
            QL_REQUIRE(isInitialized_ == true,
                       "GeneralMonteCarlo must be initialized");
            for(long j = 1; j <= iterations; j++){
                sampleAccumulator_.add(sampleGenerator_.next(),
                                       sampleGenerator_.weight());
            }
        }

        template<class SA, class SG>
        inline const SA& GeneralMonteCarlo<SA, SG>::sampleAccumulator(void) const{
            return sampleAccumulator_;
        }

    }

}
#endif
