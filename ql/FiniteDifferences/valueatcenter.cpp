
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Math/array.hpp>

namespace QuantLib {

    double valueAtCenter(const Array& a) {
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1)
            return a[jmid];
        else
            return (a[jmid]+a[jmid-1])/2.0;
    }

    double firstDerivativeAtCenter(const Array& a, const Array& g) {
        QL_REQUIRE(a.size()==g.size(),
                   "firstDerivativeAtCenter: "
                   "a and g must be of the same size");
        QL_REQUIRE(a.size()>=3,
                   "firstDerivativeAtCenter: "
                   "the size of the two vectors must be at least 3");
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1)
            return (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
        else
            return (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
    }

    double secondDerivativeAtCenter(const Array& a, const Array& g) {
        QL_REQUIRE(a.size()==g.size(),
                   "secondDerivativeAtCenter: "
                   "a and g must be of the same size");
        QL_REQUIRE(a.size()>=4,
                   "secondDerivativeAtCenter: "
                   "the size of the two vectors must be at least 4");
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1) {
            double deltaPlus = (a[jmid+1]-a[jmid])/(g[jmid+1]-g[jmid]);
            double deltaMinus = (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
            double dS = (g[jmid+1]-g[jmid-1])/2.0;
            return (deltaPlus-deltaMinus)/dS;
        } else {
            double deltaPlus = (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
            double deltaMinus = (a[jmid]-a[jmid-2])/(g[jmid]-g[jmid-2]);
            return (deltaPlus-deltaMinus)/(g[jmid]-g[jmid-1]);
        }
    }

}


