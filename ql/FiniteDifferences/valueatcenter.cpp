/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Math/array.hpp>

namespace QuantLib {

    Real valueAtCenter(const Array& a) {
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1)
            return a[jmid];
        else
            return (a[jmid]+a[jmid-1])/2.0;
    }

    Real firstDerivativeAtCenter(const Array& a, const Array& g) {
        QL_REQUIRE(a.size()==g.size(),
                   "a and g must be of the same size");
        QL_REQUIRE(a.size()>=3,
                   "the size of the two vectors must be at least 3");
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1)
            return (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
        else
            return (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
    }

    Real secondDerivativeAtCenter(const Array& a, const Array& g) {
        QL_REQUIRE(a.size()==g.size(),
                   "a and g must be of the same size");
        QL_REQUIRE(a.size()>=4,
                   "the size of the two vectors must be at least 4");
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1) {
            Real deltaPlus = (a[jmid+1]-a[jmid])/(g[jmid+1]-g[jmid]);
            Real deltaMinus = (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
            Real dS = (g[jmid+1]-g[jmid-1])/2.0;
            return (deltaPlus-deltaMinus)/dS;
        } else {
            Real deltaPlus = (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
            Real deltaMinus = (a[jmid]-a[jmid-2])/(g[jmid]-g[jmid-2]);
            return (deltaPlus-deltaMinus)/(g[jmid]-g[jmid-1]);
        }
    }

}


