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

/*! \file averageasianpathpricer.h
    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/05 11:52:12  lballabio
    Renamed SinglePathAveragePriceAsianPricer to AverageAsianPathPricer

    Revision 1.2  2001/01/05 11:42:37  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.
    
*/

#ifndef quantlib_montecarlo_average_asian_path_pricer_h
#define quantlib_montecarlo_average_asian_path_pricer_h

#include "qldefines.h"
#include "europeanpathpricer.h"

namespace QuantLib {

    namespace MonteCarlo {

        class AverageAsianPathPricer : public EuropeanPathPricer {
        public:
            AverageAsianPathPricer() {}
            AverageAsianPathPricer(Option::Type type, 
                double underlying, double strike, double discount);
            virtual double value(const Path &path) const;
        };
        
    }

}


#endif
