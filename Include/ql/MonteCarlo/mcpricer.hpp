
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

/*! \file mcpricer.hpp

    \fullpath
    Include/ql/MonteCarlo/%mcpricer.hpp
    \brief base class for one-factor Monte Carlo pricers

*/

// $Id$
// $Log$
// Revision 1.17  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.16  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.15  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.14  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.13  2001/07/27 14:45:23  marmar
// Method  void ddSamples(long n) added to GeneralMonteCarlo
//
// Revision 1.12  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.11  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.10  2001/07/13 14:29:08  sigmud
// removed a few gcc compile warnings
//
// Revision 1.9  2001/07/13 13:13:14  aleppo
// moved constructor from public to  protected
//
// Revision 1.8  2001/07/05 12:35:09  enri
// - added some static_cast<int>() to prevent gcc warnings
// - added some virtual constructor (same reason)
//
// Revision 1.7  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.6  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "ql/MonteCarlo/onefactormontecarlooption.hpp"

namespace QuantLib {

    namespace Pricers {

        //! base class for one-factor Monte Carlo pricers
        /*! Eventually this class might be linked to the general tree of 
            pricers, in order to have tools like impliedVolatility available.
            Also, it will, eventually, implement the calculation of greeks
            in Monte Carlo methods.
            Deriving a class from McPricer gives an easy way to write
            a one-factor Monte Carlo Pricer.
            See McEuropeanPricer as an example.
        */
        class McPricer {
          public:
            McPricer() : isInitialized_(false){}
            virtual ~McPricer(){}
            virtual double value() const;
            virtual double errorEstimate() const;
          protected:
            McPricer(long samples, long seed=0);
            mutable long samples_;
            long seed_;
            bool isInitialized_;
            mutable MonteCarlo::OneFactorMonteCarloOption montecarloPricer_;
        };


        // inline definitions
        
        inline McPricer::McPricer(long samples, long seed):
                    samples_(samples), seed_(seed), isInitialized_(true){}

        inline double McPricer::value() const{
            QL_REQUIRE(isInitialized_,
                "McPricer::value has not been initialized");
            montecarloPricer_.addSamples(samples_);
            return montecarloPricer_.sampleAccumulator().mean();
        }

        inline double McPricer::errorEstimate() const {
            QL_REQUIRE(isInitialized_,
                "McPricer::errorEstimate has not been initialized");
            return montecarloPricer_.sampleAccumulator().errorEstimate();
        }

    }

}

#endif
