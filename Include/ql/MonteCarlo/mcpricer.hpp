
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

    $Source$
    $Log$
    Revision 1.5  2001/05/24 13:57:51  nando
    smoothing #include xx.hpp and cutting old Log messages

    Revision 1.4  2001/05/24 12:52:02  nando
    smoothing #include xx.hpp

    Revision 1.3  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

*/

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "ql/MonteCarlo/onefactormontecarlooption.hpp"

namespace QuantLib {

    namespace Pricers {
        //! Base class for one-factor Monte Carlo pricers
        /*! McPricer is the base class for the Monte Carlo pricers depending
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
            McPricer(long samples, long seed=0);
            ~McPricer(){}
            virtual double value() const;
            virtual double errorEstimate() const;
        protected:
            bool isInitialized_;
            long seed_;
            mutable long samples_;
            mutable MonteCarlo::OneFactorMonteCarloOption montecarloPricer_;
        };

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
