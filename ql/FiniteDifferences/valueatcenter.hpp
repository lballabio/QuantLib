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

/*! \file valueatcenter.hpp
    \brief compute value, first, and second derivatives at grid center
*/

#ifndef quantlib_finite_difference_value_at_center_h
#define quantlib_finite_difference_value_at_center_h

#include <ql/Math/array.hpp>

namespace QuantLib {

    /*! mid-point value

        \todo replace with a more general (not "centered") function:
              valueAt(Real spot, const Array& a);
    */
    
    template <class T>
    Real valueAtCenter(const T& a) {
        Size jmid = a.size()/2;
        if (a.size() % 2 == 1)
            return a[jmid];
        else
            return (a[jmid]+a[jmid-1])/2.0;
    }

    /*! mid-point first derivative

        \todo replace with a more general (not "centered") function:
              firstDerivativeAt(Real spot,
                                const Array& a,
                                const Array& grid);
    */
    template <class T>
    Real firstDerivativeAtCenter(const T& a, const T& g) {
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


    /*! mid-point second derivative

        \todo replace with a more general (not "centered") function:
              secondDerivativeAt(Real spot,
                                 const Array& a,
                                 const Array& grid);
    */
    template <class T>
    Real secondDerivativeAtCenter(const T& a, const T& g) {
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


#endif
