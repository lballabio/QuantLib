
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

/*! \file valueatcenter.cpp
    \fullpath Sources/FiniteDifferences/%valueatcenter.cpp
    \brief compute value, first, and second derivatives at grid center

    $Id$
*/

// $Source$
// $Log$
// Revision 1.11  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.8  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/array.hpp"

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


