/*
 * Copyright (C) 2000
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file mcpricer.h
    
    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.2  2001/01/05 11:42:37  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.
    
*/

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "qldefines.h"
#include "montecarlo1d.h"

namespace QuantLib {

    namespace Pricers {
        //! Base class for one dimensional Monte Carlo pricers
        /*! McPricer is the base class for one dimensional Monte Carlo pricers.
            Eventually it might be linked to the general tree of pricers,
            in order to have available tools like impliedVolaitlity.
            Also, it will, eventually, implement the calculation of greeks
            in montecarlo methods.
            Deriving a class from McPricer gives an easy way to write
            a Monte Carlo Pricer.
            See EuropeanPathPricer as an example
    	*/	

        using MonteCarlo::MonteCarlo1D;

        class McPricer {
        public:
            McPricer():isInitialized_(false){}
            McPricer(long samples, long seed=0);
            ~McPricer(){}
            virtual double value() const;
            virtual double errorEstimate() const;
        protected:        
            bool isInitialized_;
            long seed_;
            mutable long samples_;
            mutable MonteCarlo1D montecarloPricer_;
        };

        inline McPricer::McPricer(long samples, long seed):
                    samples_(samples), seed_(seed), isInitialized_(true){}

        inline double McPricer::value() const{
            QL_REQUIRE(isInitialized_,
                "McPricer::value has not been initialized");
            return montecarloPricer_.value(samples_);
        }
        
        inline double McPricer::errorEstimate() const { 
            QL_REQUIRE(isInitialized_,
                "McPricer::errorEstimate has not been initialized");
            return montecarloPricer_.errorEstimate(); 
        }

    }

}


#endif
