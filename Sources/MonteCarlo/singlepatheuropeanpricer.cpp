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

/*! \file singlepatheuropeanpricer.cpp
    
    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/01/05 11:02:38  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.
                
*/

#include "singlepatheuropeanpricer.h"    
#include "qlerrors.h"


namespace QuantLib { 

    namespace MonteCarlo {

        SinglePathEuropeanPricer::SinglePathEuropeanPricer(
            Option::Type optiontype, double underlying, double strike, 
            double discount)
        : optionType_(optiontype),underlying_(underlying), strike_(strike),
            discount_(discount) {
            QL_REQUIRE(strike_ > 0.0, 
                "SinglePathEuropeanPricer: strike must be positive");              
            QL_REQUIRE(underlying_ > 0.0, 
                "SinglePathEuropeanPricer: underlying must be positive");            
            QL_REQUIRE(discount_ > 0.0, 
                "SinglePathEuropeanPricer: discount must be positive");            
            isInitialized_ = true;
        }

        double SinglePathEuropeanPricer::value(const Path & path) const {
            int n = path.size();
            QL_REQUIRE(isInitialized_,
                "SinglePathEuropeanPricer: pricer not initialized");
            QL_REQUIRE(n>0,
                "SinglePathEuropeanPricer: the path cannot be empty");

            double price = underlying_;
            for(unsigned int i = 0; i < n; i++)
                price *= QL_EXP(path[i]);
            
            return computePlainVanilla(optionType_, price, strike_, discount_);
        }

        double SinglePathEuropeanPricer::computePlainVanilla(
          Option::Type optiontype, double price, double strike, 
          double discount) const {
            double optionPrice;
            switch (optiontype) {
              case Option::Call:
                    optionPrice = QL_MAX(price-strike,0.0);
                break;
              case Option::Put:
                    optionPrice = QL_MAX(strike-price,0.0);
                break;
              case Option::Straddle:
                    optionPrice = QL_FABS(strike-price);
            }
            return discount*optionPrice;
        }

    }

}

