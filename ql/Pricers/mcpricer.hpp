
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
    \brief base class for Monte Carlo pricers
     
    \fullpath
    ql/Pricers/%mcpricer.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "ql/MonteCarlo/montecarlomodel.hpp"

namespace QuantLib {

    namespace Pricers {

        //! base class for Monte Carlo pricers
        /*! Eventually this class might be linked to the general tree of 
            pricers, in order to have tools like impliedVolatility available.
            Also, it will, eventually, offer greeks methods.
            Deriving a class from McPricer gives an easy way to write
            a Monte Carlo Pricer.
            See McEuropeanPricer as an example of one factor pricer,
            EverestOption as an example of multi factor pricer.
        */
        template<class S, class PG, class PP>
        class McPricer {
          public:
            virtual ~McPricer() {}
            virtual double value() const;
            virtual double errorEstimate() const;
          protected:
            McPricer() {};
            mutable Handle<MonteCarlo::MonteCarloModel<S, PG, PP> > mcModel_;
        };


        // inline definitions
        
        template<class S, class PG, class PP>
        inline double McPricer<S, PG, PP>::value() const {
            QL_REQUIRE(mcModel_->sampleAccumulator().samples()>0, 
                "No simulated samples yet");
            return mcModel_->sampleAccumulator().mean();
        }

        template<class S, class PG, class PP>
        inline double McPricer<S, PG, PP>::errorEstimate() const {
            QL_REQUIRE(mcModel_->sampleAccumulator().samples()>0, 
                "No simulated samples yet");
            return mcModel_->sampleAccumulator().errorEstimate();
        }

    }

}

#endif
