/*
 * Copyright (C) 2000, 2001
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

/*! \file europeanpathpricer.h
    
    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/02/02 10:48:10  marmar
    Destructor already implemented in base class

    Revision 1.2  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.1  2001/01/05 11:42:37  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.2  2001/01/05 11:02:37  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.
                
*/

#ifndef quantlib_montecarlo_european_path_pricer_h
#define quantlib_montecarlo_european_path_pricer_h

#include "pathpricer.h"
#include "options.h"

namespace QuantLib {

    namespace MonteCarlo {
        /*! EuropeanPathPricer evaluates the european-option value on a 
            single-path. 
            The public method computePlainVanilla can also be used 
            in other path pricer that do similar calculations.
        */

        class EuropeanPathPricer : public PathPricer {
          public:
            EuropeanPathPricer():PathPricer(){}
            EuropeanPathPricer(Option::Type type, double underlying, 
                double strike, double discount);
            double value(const Path &path) const;
            double computePlainVanilla(Option::Type type, double price, 
                double strike, double discount) const;
          protected:
            mutable Option::Type type_;
            mutable double underlying_, strike_, discount_;
        };

    }

}


#endif
