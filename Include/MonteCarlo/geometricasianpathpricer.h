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

/*! \file geometricasianpathpricer.h
    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/05 11:18:03  lballabio
    Renamed SinglePathGeometricAsianPricer to GeometricAsianPathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.
    
*/


#ifndef quantlib_montecarlo_geometric_asian_pricer_h
#define quantlib_montecarlo_geometric_asian_pricer_h

#include "qldefines.h"
#include "singlepatheuropeanpricer.h"

namespace QuantLib {

    namespace MonteCarlo {

        class GeometricAsianPathPricer : public SinglePathEuropeanPricer{
          public:
            GeometricAsianPathPricer() : SinglePathEuropeanPricer() {}
            GeometricAsianPathPricer(Option::Type type, double underlying,
                double strike, double discount);
            virtual double value(const Path &path) const;
        };
        
    }

}


#endif
