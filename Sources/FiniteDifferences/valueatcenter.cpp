
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
 * QuantLib license is also available at 
 *  http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file valueatcenter.cpp
    \brief compute value, first, and second derivatives at grid center

    $Source$
    $Log$
    Revision 1.1  2001/03/21 10:47:37  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions


*/

#include "bsmnumericaloption.h"

namespace QuantLib {

    namespace FiniteDifferences {
                
        double valueAtCenter(const Array& a) {
            int jmid = a.size()/2;
            if (a.size() % 2 == 1)
                return a[jmid];
            else 
                return (a[jmid]+a[jmid-1])/2.0;
        }
            
        double firstDerivativeAtCenter(const Array& a, const Array& g) {
            QL_REQUIRE(a.size()==g.size(),
                "BSMNumericalOption::firstDerivativeAtCenter: "
                "a and g must be of the same size");
            QL_REQUIRE(a.size()>=3,
                "BSMNumericalOption::firstDerivativeAtCenter: "
                "the size of the two vectors must be at least 3");
            int jmid = a.size()/2;
            if(a.size() % 2 == 1)
                return (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
            else
                return (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
        }
        
        double secondDerivativeAtCenter(const Array& a, const Array& g) {
            QL_REQUIRE(a.size()==g.size(),
                "BSMNumericalOption::secondDerivativeAtCenter: "
                "a and g must be of the same size");
            QL_REQUIRE(a.size()>=4,
                "BSMNumericalOption::secondDerivativeAtCenter: "
                "the size of the two vectors must be at least 4");
            int jmid = a.size()/2;
            if(a.size() % 2 == 1){
                double deltaPlus = (a[jmid+1]-a[jmid])/(g[jmid+1]-g[jmid]);
                double deltaMinus = (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
                double dS = (g[jmid+1]-g[jmid-1])/2.0;
                return (deltaPlus-deltaMinus)/dS;    
            }          
            else{
                double deltaPlus = (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
                double deltaMinus = (a[jmid]-a[jmid-2])/(g[jmid]-g[jmid-2]);
                return (deltaPlus-deltaMinus)/(g[jmid]-g[jmid-1]);    
            }
        }
    }
}


