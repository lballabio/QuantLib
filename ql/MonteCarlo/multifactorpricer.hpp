
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

/*! \file multifactorpricer.hpp
    \brief base class for multi-factor Monte Carlo pricers

    \fullpath
    ql/MonteCarlo/%multifactorpricer.hpp

*/

// $Id$

#ifndef quantlib_montecarlo_multi_factor_pricer_h
#define quantlib_montecarlo_multi_factor_pricer_h

#include "ql/MonteCarlo/mctypedefs.hpp"

namespace QuantLib {

    namespace Pricers {

        //! base class for multi-factor Monte Carlo pricers
        /*! Eventually this class might be linked to the general tree of 
            pricers, in order to have tools like impliedVolatility available. 
            Also, it will, eventually, implement the calculation of greeks in 
            Monte Carlo methods.
            Deriving a class from MultiFactorPricer gives an easy way to 
            write a multi-factor Monte Carlo Pricer.
            See PlainBasketOption for an example.
        */
        class MultiFactorPricer {
          public:
            MultiFactorPricer(long samples, long seed=0);
            virtual ~MultiFactorPricer(){}
            virtual double value() const;
            virtual double errorEstimate() const;
          protected:
            long seed_;
            mutable long samples_;
            mutable Handle<MonteCarlo::MultiFactorMonteCarloOption> montecarloPricer_;
        };

        // inline definitions
        
        inline MultiFactorPricer::MultiFactorPricer(long samples, long seed):
                    seed_(seed), samples_(samples) {}

        inline double MultiFactorPricer::value() const{
            montecarloPricer_->addSamples(samples_);
            return montecarloPricer_->sampleAccumulator().mean();
        }

        inline double MultiFactorPricer::errorEstimate() const {
            return montecarloPricer_->sampleAccumulator().errorEstimate();
        }

    }

}

#endif
