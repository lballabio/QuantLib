
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
    \brief base class for one-factor Monte Carlo pricers
     
    \fullpath
    ql/MonteCarlo/%mcpricer.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "ql/MonteCarlo/mctypedefs.hpp"

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
            virtual ~McPricer(){}
            virtual double value() const;
            virtual double errorEstimate() const;
          protected:
            McPricer(long samples, long seed=0);
            mutable long samples_;
            long seed_;
            mutable Handle<MonteCarlo::OneFactorMonteCarloOption> montecarloPricer_;
        };


        // inline definitions
        
        inline McPricer::McPricer(long samples, long seed):
                    samples_(samples), seed_(seed) {}

        inline double McPricer::value() const{
            montecarloPricer_->addSamples(samples_);
            return montecarloPricer_->sampleAccumulator().mean();
        }

        inline double McPricer::errorEstimate() const {
            return montecarloPricer_->sampleAccumulator().errorEstimate();
        }

    }

}

#endif
