
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

/*! \file mcpricer.hpp

    $Id$
*/

// $Source$
// $Log$
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

        //! Base class for one-factor Monte Carlo pricers
        /*! Base class for the Monte Carlo pricers depending
            from one factor.
            Eventually it might be linked to the general tree of pricers,
            in order to have available tools like impliedVolaitlity.
            Also, it will, eventually, implement the calculation of greeks
            in montecarlo methods.
            Deriving a class from McPricer gives an easy way to write
            a one-factor Monte Carlo Pricer.
            See McEuropeanPricer as an example
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
            return montecarloPricer_.sampleAccumulator(samples_).mean();
        }

        inline double McPricer::errorEstimate() const {
            QL_REQUIRE(isInitialized_,
                "McPricer::errorEstimate has not been initialized");
            return montecarloPricer_.sampleAccumulator().errorEstimate();
        }

    }

}

#endif
